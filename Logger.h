#pragma once

#include "btstack.h"

// Logs progress messages over both USB serial and BLE (a Nordic UART Service peripheral), and
// receives short text commands back over either transport. This is the one place in the
// codebase that touches USB stdio or BTstack, so the rest of the robot logic stays free of
// serial/BLE plumbing - main() just calls Log()/Logf() and polls HasCommand()/TakeCommand().
class Logger
{
private:
    static constexpr int CommandBufferSize = 32;
    static constexpr int MaxLogMessageLength = 128;

    static Logger *activeInstance; // one instance, used to route BTstack's C-style callbacks into it

    hci_con_handle_t bleConnectionHandle;
    btstack_context_callback_registration_t bleSendRequest;

    char pendingBleMessage[MaxLogMessageLength];
    int pendingBleLength;
    int pendingBleOffset;
    bool bleSendInFlight;

    char inProgressCommand[CommandBufferSize];
    int inProgressLength;
    char completedCommand[CommandBufferSize];
    bool commandPending;

    static void PacketHandlerTrampoline(uint8_t packetType, uint16_t channel, uint8_t *packet, uint16_t size);
    void HandlePacket(uint8_t packetType, uint16_t channel, uint8_t *packet, uint16_t size);

    static void SendNextChunkTrampoline(void *context);
    void SendNextChunk();

    void PollUsbSerial();
    void AcceptCommandBytes(const char *data, int length);

public:
    Logger();

    void Init();   // brings up cyw43/BTstack and starts advertising as Config.h's BleDeviceName
    void Update(); // call every main-loop iteration - pumps non-blocking USB RX

    void Log(const char *message);
    void Logf(const char *format, ...);

    bool HasCommand() const { return commandPending; }
    const char *TakeCommand(); // returns the pending command and clears HasCommand()
};
