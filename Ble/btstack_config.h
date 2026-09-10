#ifndef _PICO_BTSTACK_BTSTACK_CONFIG_H
#define _PICO_BTSTACK_BTSTACK_CONFIG_H

// BLE only - no classic Bluetooth (SPP/BR-EDR) needed for a logging/command link, so this is
// trimmed down from the Pico SDK's example config to just what BLE peripheral mode uses.
#define ENABLE_LOG_INFO
#define ENABLE_LOG_ERROR
#define ENABLE_PRINTF_HEXDUMP

// ENABLE_BLE itself is defined automatically by linking pico_btstack_ble - not set here
#ifdef ENABLE_BLE
#define ENABLE_LE_PERIPHERAL
#define ENABLE_LE_DATA_LENGTH_EXTENSION
#endif

// BTstack configuration - buffers, sizes, ...
#define HCI_OUTGOING_PRE_BUFFER_SIZE 4
#define HCI_ACL_PAYLOAD_SIZE (1691 + 4)
#define HCI_ACL_CHUNK_SIZE_ALIGNMENT 4
#define MAX_NR_HCI_CONNECTIONS 1
#define MAX_NR_L2CAP_CHANNELS 4
#define MAX_NR_L2CAP_SERVICES 3
#define MAX_NR_LE_DEVICE_DB_ENTRIES 1

// Link key/device DB storage backing (TLV over flash) - pulled in transitively by
// pico_btstack_cyw43, needed even without bonding
#define NVM_NUM_DEVICE_DB_ENTRIES 1

// Limit number of ACL buffers to use by stack to avoid cyw43 shared bus overrun
#define MAX_NR_CONTROLLER_ACL_BUFFERS 3

// Enable and configure HCI Controller to Host Flow Control to avoid cyw43 shared bus overrun
#define ENABLE_HCI_CONTROLLER_TO_HOST_FLOW_CONTROL
#define HCI_HOST_ACL_PACKET_LEN 1024
#define HCI_HOST_ACL_PACKET_NUM 3
// no SCO (classic voice) in a BLE-only build, but the flow control code still wants these defined
#define HCI_HOST_SCO_PACKET_LEN 0
#define HCI_HOST_SCO_PACKET_NUM 0

// We don't give btstack a malloc, so use a fixed-size ATT DB
#define MAX_ATT_DB_SIZE 512

// BTstack HAL configuration
#define HAVE_EMBEDDED_TIME_MS

// map btstack_assert onto Pico SDK assert()
#define HAVE_ASSERT

#define ENABLE_SOFTWARE_AES128
#define ENABLE_MICRO_ECC_FOR_LE_SECURE_CONNECTIONS

#endif // _PICO_BTSTACK_BTSTACK_CONFIG_H
