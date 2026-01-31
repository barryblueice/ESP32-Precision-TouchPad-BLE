
#include "hidd_le_prf_int.h"
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"

#include "ble/ble.h"

#define HID_RPT_ID_TOUCH_IN      1 
#define HID_TOUCH_IN_RPT_LEN     63

esp_err_t esp_hidd_register_callbacks(esp_hidd_event_cb_t callbacks)
{
    esp_err_t hidd_status;
    if(callbacks != NULL) {
        hidd_le_env.hidd_cb = callbacks;
    } else {
        return ESP_FAIL;
    }

    if((hidd_status = hidd_register_cb()) != ESP_OK) {
        return hidd_status;
    }

    esp_ble_gatts_app_register(BATTRAY_APP_ID);
    if((hidd_status = esp_ble_gatts_app_register(HIDD_APP_ID)) != ESP_OK) {
        return hidd_status;
    }

    return hidd_status;
}

esp_err_t esp_hidd_profile_init(void)
{
     if (hidd_le_env.enabled) {
        ESP_LOGE(HID_LE_PRF_TAG, "HID device profile already initialized");
        return ESP_FAIL;
    }
    memset(&hidd_le_env, 0, sizeof(hidd_le_env_t));
    hidd_le_env.enabled = true;
    return ESP_OK;
}

void esp_hidd_send_touchpad_value(uint16_t conn_id, uint8_t *data, uint16_t length)
{
    if (!hidd_le_env.enabled) {
        return;
    }
    
    hid_dev_send_report(hidd_le_env.gatt_if, conn_id,
                        HID_RPT_ID_TOUCH_IN, HID_REPORT_TYPE_INPUT, length, data);
}

void hidd_le_gatts_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
        case ESP_GATTS_REG_EVT: {
            break;
        }
        case ESP_GATTS_READ_EVT: {
            break;
        }
        case ESP_GATTS_WRITE_EVT: {
            break;
        }
        case ESP_GATTS_CONNECT_EVT: {
            hidd_le_env.conn_id = param->connect.conn_id;
            break;
        }
        default:
            break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param)
{
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            hidd_le_env.gatt_if = gatts_if;
        } else {
            ESP_LOGE(HID_LE_PRF_TAG, "Reg app failed, app_id %04x, status %d",
                     param->reg.app_id, param->reg.status);
            return;
        }
    }
    extern void hidd_le_gatts_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
    
    hidd_le_gatts_cb(event, gatts_if, param);
}

esp_err_t hidd_register_cb(void)
{
    esp_err_t status;
    status = esp_ble_gatts_register_callback(gatts_event_handler);
    return status;
}