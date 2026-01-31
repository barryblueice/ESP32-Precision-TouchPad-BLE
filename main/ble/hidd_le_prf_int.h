#ifndef __HID_DEVICE_LE_PRF__
#define __HID_DEVICE_LE_PRF__

#include "esp_gatts_api.h"
#include "ble/ble.h"

#define HID_LE_PRF_TAG "PTP_PRF"

#define HIDD_VERSION 0x0100

#define HID_REPORT_TYPE_INPUT   1
#define HID_REPORT_TYPE_OUTPUT  2
#define HID_REPORT_TYPE_FEATURE 3

#define HID_RPT_ID_TOUCH_IN      1
#define HID_RPT_ID_FEATURE       4 

enum {
    HIDD_LE_IDX_SVC,
    HIDD_LE_IDX_HID_INFO_CHAR,
    HIDD_LE_IDX_HID_INFO_VAL,
    HIDD_LE_IDX_HID_CTNL_PT_CHAR,
    HIDD_LE_IDX_HID_CTNL_PT_VAL,
    HIDD_LE_IDX_REPORT_MAP_CHAR,
    HIDD_LE_IDX_REPORT_MAP_VAL,
    
    HIDD_LE_IDX_REPORT_TOUCH_IN_CHAR,
    HIDD_LE_IDX_REPORT_TOUCH_IN_VAL,
    HIDD_LE_IDX_REPORT_TOUCH_IN_CCC,
    HIDD_LE_IDX_REPORT_TOUCH_REP_REF,

    HIDD_LE_IDX_REPORT_PTP_FEATURE_CHAR,
    HIDD_LE_IDX_REPORT_PTP_FEATURE_VAL,
    HIDD_LE_IDX_REPORT_PTP_FEATURE_REP_REF,

    HIDD_LE_IDX_NB,
};

#define BATTRAY_APP_ID       0x180f

#define HIDD_APP_ID			0x1812

typedef struct {
    esp_gatt_if_t       gatt_if;
    uint16_t            conn_id;
    bool                connected;
    esp_bd_addr_t       remote_bda;
    bool                enabled;
    esp_hidd_event_cb_t hidd_cb;
    uint16_t            att_tbl[HIDD_LE_IDX_NB];
} hidd_le_env_t;

extern hidd_le_env_t hidd_le_env;

esp_err_t hidd_register_cb(void);

typedef struct {
    uint16_t handle;      // 属性句柄
    uint16_t cccdHandle;  // 通知句柄
    uint8_t  id;          // Report ID
    uint8_t  type;        // Input/Feature
} hid_report_map_t;

/**
 * @brief 发送 HID 报文的通用函数
 */
void hid_dev_send_report(esp_gatt_if_t gatts_if, uint16_t conn_id,
                         uint8_t id, uint8_t type, uint8_t length, uint8_t *data);

#endif