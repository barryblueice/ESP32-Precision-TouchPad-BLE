#include "nimble/nimble_port.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include <stdio.h>
#include "esp_system.h"
#include "esp_mac.h"

static const char *TAG = "BLE_GATT_INIT";

extern const struct ble_gatt_svc_def gatt_svr_svcs[];

void ble_host_task(void *param) {
    nimble_port_run();
    nimble_port_deinit();
    vTaskDelete(NULL);
}

static uint8_t addr_type;

static int ble_app_gap_event(struct ble_gap_event *event, void *arg);

void ble_app_advertise(void) {
    struct ble_hs_adv_fields fields;
    int rc;

    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    fields.name = (uint8_t *)"ESP32_Touchpad";
    fields.name_len = strlen("ESP32_Touchpad");
    fields.name_is_complete = 1;

    fields.appearance = 0x03C5;
    fields.appearance_is_present = 1;

    fields.uuids16 = (ble_uuid16_t[]){ BLE_UUID16_INIT(0x1812) };
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        printf("Error setting advertisement data; rc=%d\n", rc);
        return;
    }

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    
    rc = ble_gap_adv_start(addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, ble_app_gap_event, NULL);
    if (rc != 0) {
        printf("Error starting advertisement; rc=%d\n", rc);
    }
}

static int ble_app_gap_event(struct ble_gap_event *event, void *arg) {
    int rc;

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            printf("Connected! status=%d\n", event->connect.status);
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            printf("Disconnected! reason=%d\n", event->disconnect.reason);
            ble_app_advertise();
            break;

        case BLE_GAP_EVENT_PASSKEY_ACTION:
            printf("PASSKEY_ACTION! action=%d\n", event->passkey.params.action);
            
            struct ble_sm_io pkey;
            memset(&pkey, 0, sizeof(pkey));

            if (event->passkey.params.action == BLE_SM_IOACT_NUMCMP) {
                pkey.action = event->passkey.params.action;
                pkey.numcmp_accept = 1;
                rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
                printf("Numeric Comparison accepted; rc=%d\n", rc);
            } else if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
                pkey.action = event->passkey.params.action;
                pkey.passkey = 123456; 
                rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
                printf("Passkey injected; rc=%d\n", rc);
            }
            break;

        case BLE_GAP_EVENT_ENC_CHANGE:
            printf("Encryption Change! status=%d\n", event->enc_change.status);
            break;

        case BLE_GAP_EVENT_REPEAT_PAIRING:
            printf("Repeat pairing requested. Deleting old bond...\n");
            struct ble_gap_conn_desc desc;
            ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
            ble_store_util_delete_peer(&desc.peer_id_addr);
            return BLE_GAP_REPEAT_PAIRING_RETRY;
        
        case BLE_GAP_EVENT_MTU:
            printf("MTU updated to %d; NOW requesting security...\n", event->mtu.value);
            break;
    }
    return 0;
}

static void ble_app_on_sync(void) {
    ble_hs_id_infer_auto(0, &addr_type);
    ble_app_advertise();
}

void ble_gatt_init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    nimble_port_init();

    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_DISP_ONLY;
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm = 1;
    ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ID | BLE_SM_PAIR_KEY_DIST_ENC;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ID | BLE_SM_PAIR_KEY_DIST_ENC;

    ble_svc_gap_init();
    ble_svc_gatt_init();
    
    ble_gatts_count_cfg(gatt_svr_svcs);
    ble_gatts_add_svcs(gatt_svr_svcs);

    ble_hs_cfg.sync_cb = ble_app_on_sync;

    xTaskCreate(ble_host_task, "ble_host", 4096, NULL, 12, NULL);
}