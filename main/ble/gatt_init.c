#include "nimble/nimble_port.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

extern const struct ble_gatt_svc_def gatt_svr_svcs[];

void ble_host_task(void *param) {
    nimble_port_run();
    nimble_port_deinit();
    vTaskDelete(NULL);
}

void ble_app_on_sync(void) {
    uint8_t addr_type;
    ble_hs_id_infer_auto(0, &addr_type);

    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    
    fields.appearance = 0x03C5;
    fields.appearance_is_present = 1;

    fields.name = (uint8_t *)CONFIG_BLE_DEVICE_NAME;
    fields.name_len = strlen(CONFIG_BLE_DEVICE_NAME);
    fields.name_is_complete = 1;

    fields.uuids16 = (ble_uuid16_t[]){ BLE_UUID16_INIT(0x1812) };
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    ble_gap_adv_start(addr_type, NULL, BLE_HS_FOREVER, &adv_params, NULL, NULL);
}

void ble_gatt_init() {
    nvs_flash_init();
    nimble_port_init();

    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm = 0;
    ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;

    ble_svc_gap_init();
    ble_svc_gatt_init();
    
    ble_gatts_count_cfg(gatt_svr_svcs);
    ble_gatts_add_svcs(gatt_svr_svcs);

    ble_hs_cfg.sync_cb = ble_app_on_sync;

    xTaskCreate(ble_host_task, "ble_host", 4096, NULL, 12, NULL);
}