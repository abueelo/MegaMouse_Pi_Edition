#include "Logger.h"
#include "Config.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "ble/gatt-service/nordic_spp_service_server.h"
#include "MegaMouseBle.h" // generated from Ble/MegaMouseBle.gatt by pico_btstack_make_gatt_header

#include <cstdio>
#include <cstdarg>
#include <cstring>

Logger *Logger::activeInstance = nullptr;

Logger::Logger()
    : bleConnectionHandle(HCI_CON_HANDLE_INVALID),
      bleSendRequest(),
      pendingBleLength(0),
      pendingBleOffset(0),
      bleSendInFlight(false),
      inProgressLength(0),
      commandPending(false)
{
    pendingBleMessage[0] = '\0';
    inProgressCommand[0] = '\0';
    completedCommand[0] = '\0';
    activeInstance = this;
}

void Logger::Init()
{
    if (cyw43_arch_init())
    {
        // BLE won't be available, but USB logging still works fine without it
        printf("Logger: cyw43_arch_init failed, BLE logging disabled\n");
        return;
    }

    l2cap_init();
    sm_init();
    sm_set_authentication_requirements(0); // no bonding/pairing needed for a hobby telemetry link

    att_server_init(profile_data, nullptr, nullptr);
    nordic_spp_service_server_init(&Logger::PacketHandlerTrampoline);

    // Flags (general discoverable, BR/EDR not supported) + complete local name
    static uint8_t advData[31];
    int len = 0;
    advData[len++] = 2;
    advData[len++] = BLUETOOTH_DATA_TYPE_FLAGS;
    advData[len++] = 0x06;

    int nameLen = static_cast<int>(strlen(BleDeviceName));
    advData[len++] = static_cast<uint8_t>(nameLen + 1);
    advData[len++] = BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME;
    memcpy(&advData[len], BleDeviceName, nameLen);
    len += nameLen;

    uint16_t advIntMin = 0x0030;
    uint16_t advIntMax = 0x0030;
    bd_addr_t nullAddr;
    memset(nullAddr, 0, 6);
    gap_advertisements_set_params(advIntMin, advIntMax, 0, 0, nullAddr, 0x07, 0x00);
    gap_advertisements_set_data(static_cast<uint8_t>(len), advData);
    gap_advertisements_enable(1);

    hci_power_control(HCI_POWER_ON);

    // Deliberately no btstack_run_loop_execute() call - BTstack's processing is dispatched by
    // the same background hardware IRQ that pico_cyw43_arch_none already runs on (confirmed by
    // reading async_context_threadsafe_background.c), so it keeps working regardless of what
    // the main loop does. Update() still calls cyw43_arch_poll() every iteration as zero-cost
    // insurance against that assumption ever being wrong.
}

void Logger::Update()
{
    PollUsbSerial();
    cyw43_arch_poll();
}

void Logger::PollUsbSerial()
{
    while (true)
    {
        int c = getchar_timeout_us(0);
        if (c == PICO_ERROR_TIMEOUT)
        {
            break;
        }
        char ch = static_cast<char>(c);
        AcceptCommandBytes(&ch, 1);
    }
}

void Logger::AcceptCommandBytes(const char *data, int length)
{
    for (int i = 0; i < length; i++)
    {
        char c = data[i];
        if (c == '\n' || c == '\r')
        {
            if (inProgressLength > 0)
            {
                inProgressCommand[inProgressLength] = '\0';
                strcpy(completedCommand, inProgressCommand);
                commandPending = true;
                inProgressLength = 0;
            }
            continue;
        }

        if (inProgressLength < CommandBufferSize - 1)
        {
            inProgressCommand[inProgressLength++] = c;
        }
    }
}

const char *Logger::TakeCommand()
{
    commandPending = false;
    return completedCommand;
}

void Logger::Log(const char *message)
{
    printf("%s\n", message);

    if (bleConnectionHandle == HCI_CON_HANDLE_INVALID)
    {
        return;
    }

    // Best-effort mirror over BLE: if a previous line is still being sent when a new one comes
    // in, the new one replaces it rather than queuing - this is a log, not a guaranteed-delivery
    // channel, and USB above already got every line.
    size_t length = strlen(message);
    if (length >= MaxLogMessageLength)
    {
        length = MaxLogMessageLength - 1;
    }
    memcpy(pendingBleMessage, message, length);
    pendingBleMessage[length] = '\0';
    pendingBleLength = static_cast<int>(length);
    pendingBleOffset = 0;

    if (!bleSendInFlight)
    {
        bleSendInFlight = true;
        bleSendRequest.callback = &Logger::SendNextChunkTrampoline;
        nordic_spp_service_server_request_can_send_now(&bleSendRequest, bleConnectionHandle);
    }
}

void Logger::Logf(const char *format, ...)
{
    char buffer[MaxLogMessageLength];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Log(buffer);
}

void Logger::SendNextChunkTrampoline(void *context)
{
    (void)context;
    if (activeInstance)
    {
        activeInstance->SendNextChunk();
    }
}

void Logger::SendNextChunk()
{
    if (bleConnectionHandle == HCI_CON_HANDLE_INVALID)
    {
        bleSendInFlight = false;
        return;
    }

    int mtuPayload = att_server_get_mtu(bleConnectionHandle) - 3;
    if (mtuPayload < 1)
    {
        mtuPayload = 20; // fallback before the MTU exchange completes
    }

    int remaining = pendingBleLength - pendingBleOffset;
    int chunkSize = remaining < mtuPayload ? remaining : mtuPayload;

    nordic_spp_service_server_send(bleConnectionHandle,
                                    reinterpret_cast<const uint8_t *>(&pendingBleMessage[pendingBleOffset]),
                                    static_cast<uint16_t>(chunkSize));
    pendingBleOffset += chunkSize;

    if (pendingBleOffset < pendingBleLength)
    {
        nordic_spp_service_server_request_can_send_now(&bleSendRequest, bleConnectionHandle);
    }
    else
    {
        bleSendInFlight = false;
    }
}

void Logger::PacketHandlerTrampoline(uint8_t packetType, uint16_t channel, uint8_t *packet, uint16_t size)
{
    if (activeInstance)
    {
        activeInstance->HandlePacket(packetType, channel, packet, size);
    }
}

void Logger::HandlePacket(uint8_t packetType, uint16_t channel, uint8_t *packet, uint16_t size)
{
    (void)channel;
    switch (packetType)
    {
    case HCI_EVENT_PACKET:
        if (hci_event_packet_get_type(packet) != HCI_EVENT_GATTSERVICE_META)
        {
            break;
        }
        switch (hci_event_gattservice_meta_get_subevent_code(packet))
        {
        case GATTSERVICE_SUBEVENT_SPP_SERVICE_CONNECTED:
            bleConnectionHandle = gattservice_subevent_spp_service_connected_get_con_handle(packet);
            break;
        case GATTSERVICE_SUBEVENT_SPP_SERVICE_DISCONNECTED:
            bleConnectionHandle = HCI_CON_HANDLE_INVALID;
            bleSendInFlight = false;
            break;
        default:
            break;
        }
        break;
    case RFCOMM_DATA_PACKET: // historical name - this is really the ATT write on the RX characteristic
        AcceptCommandBytes(reinterpret_cast<const char *>(packet), size);
        break;
    default:
        break;
    }
}
