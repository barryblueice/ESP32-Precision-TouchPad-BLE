#ifndef BLE_H
#define BLE_H

#include "esp_bt_defs.h"
#include "esp_gatt_defs.h"
#include "esp_err.h"

typedef enum {
    ESP_HIDD_EVENT_REG_FINISH = 0,
    ESP_HIDD_EVENT_BLE_CONNECT,
    ESP_HIDD_EVENT_BLE_DISCONNECT,
    ESP_HIDD_EVENT_BLE_VENDOR_REPORT_WRITE_EVT,
} esp_hidd_cb_event_t;

typedef enum {
    ESP_HIDD_INIT_OK = 0,
    ESP_HIDD_INIT_FAILED = 1,
} esp_hidd_init_state_t;

typedef union {
    struct hidd_init_finish_evt_param {
        esp_hidd_init_state_t state;
        esp_gatt_if_t gatts_if;
    } init_finish;

    struct hidd_connect_evt_param {
        uint16_t conn_id;
        esp_bd_addr_t remote_bda;
    } connect;

    struct hidd_disconnect_evt_param {
        esp_bd_addr_t remote_bda;
    } disconnect;

    struct hidd_vendor_write_evt_param {
        uint16_t conn_id;
        uint16_t report_id;
        uint16_t length;
        uint8_t  *data;
    } vendor_write;
} esp_hidd_cb_param_t;

typedef void (*esp_hidd_event_cb_t) (esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param);

esp_err_t esp_hidd_register_callbacks(esp_hidd_event_cb_t callbacks);
esp_err_t esp_hidd_profile_init(void);

void ble_gatt_init(void);

#endif