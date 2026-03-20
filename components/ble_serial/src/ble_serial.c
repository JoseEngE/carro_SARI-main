#include "ble_serial.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define TAG "BLE_SERIAL"

static uint8_t own_addr_type;
static uint16_t conn_handle;
static bool client_connected = false;
static uint16_t tx_val_handle;

// --- Nordic UART Service (NUS) UUIDs ---
static const ble_uuid128_t gatt_svr_svc_uart_uuid =
    BLE_UUID128_INIT(0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E);
static const ble_uuid128_t gatt_svr_chr_uart_rx_uuid =
    BLE_UUID128_INIT(0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E);
static const ble_uuid128_t gatt_svr_chr_uart_tx_uuid =
    BLE_UUID128_INIT(0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E);

static int gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        // Ignored for now. Allows App to send text to ESP32.
    }
    return 0;
}

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_uart_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = &gatt_svr_chr_uart_rx_uuid.u,
                .access_cb = gatt_svr_chr_access,
                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
            },
            {
                .uuid = &gatt_svr_chr_uart_tx_uuid.u,
                .access_cb = gatt_svr_chr_access,
                .flags = BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &tx_val_handle,
            },
            {
                0, /* No more characteristics */
            }
        },
    },
    {
        0, /* No more services */
    },
};

static void ble_serial_adv_start(void);

static int ble_serial_gap_event(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status == 0) {
                conn_handle = event->connect.conn_handle;
                client_connected = true;
                ESP_LOGI(TAG, "Client Connected");
            } else {
                ble_serial_adv_start();
            }
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            client_connected = false;
            ESP_LOGI(TAG, "Client Disconnected");
            ble_serial_adv_start();
            break;
    }
    return 0;
}

static void ble_serial_adv_start(void) {
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name = "Sensores-BLE";

    memset(&fields, 0, sizeof fields);
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_serial_gap_event, NULL);
}

static void ble_serial_on_sync(void) {
    ble_hs_id_infer_auto(0, &own_addr_type);
    ble_serial_adv_start();
}

static void ble_serial_host_task(void *param) {
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void ble_serial_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    nimble_port_init();

    ble_hs_cfg.sync_cb = ble_serial_on_sync;

    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svr_svcs);
    ble_gatts_add_svcs(gatt_svr_svcs);
    ble_svc_gap_device_name_set("Sensores-BLE");

    nimble_port_freertos_init(ble_serial_host_task);
    ESP_LOGI(TAG, "BLE Serial initialized");
}

void ble_serial_print(const char *format, ...) {
    if (!client_connected) return;

    char buffer[200]; // Stay under typical MTU comfortably
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0) {
        struct os_mbuf *txom = ble_hs_mbuf_from_flat(buffer, len);
        if (txom) {
            ble_gatts_notify_custom(conn_handle, tx_val_handle, txom);
        }
    }
}

bool ble_serial_is_connected(void) {
    return client_connected;
}
