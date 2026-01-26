#include "nimble/nimble_port.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/hid/ble_svc_hid.h"

uint16_t hid_tp_in_handle;
uint16_t hid_mouse_in_handle;
uint16_t hid_ptphqa_handle;
uint16_t hid_input_mode_handle;

static uint8_t ref_tp_in[]   = {0x01, 0x01};
static uint8_t ref_mouse_in[]= {0x02, 0x01};
static uint8_t ref_max_cnt[] = {0x03, 0x03};
static uint8_t ref_hqa[]     = {0x04, 0x03};
static uint8_t ref_mode[]    = {0x05, 0x03};

extern const uint8_t ble_ptp_hid_report_descriptor[];
extern const size_t ble_ptp_hid_report_descriptor_len;

uint16_t hid_max_count_handle;

static int gatt_svr_access_hid(uint16_t conn_handle, uint16_t attr_handle, 
                               struct ble_gatt_access_ctxt *ctxt, void *arg) 
{
    uint16_t uuid16 = ble_uuid_u16(ctxt->chr->uuid);

    switch (uuid16) {
        case 0x2A4A: // HID Information: bcdHID(1.11), Country(0), Flags(RemoteWake|NormallyConnectable)
            static const uint8_t hid_info[] = {0x11, 0x01, 0x00, 0x01};
            return os_mbuf_append(ctxt->om, hid_info, sizeof(hid_info));

        case 0x2A4B:
            return os_mbuf_append(ctxt->om, ble_ptp_hid_report_descriptor, ble_ptp_hid_report_descriptor_len);

        case 0x2A4D:
            if (attr_handle == hid_ptphqa_handle) {
                static uint8_t hqa[256] = {0};
                return os_mbuf_append(ctxt->om, hqa, 256);
            }
            if (attr_handle == hid_input_mode_handle) {
                static uint8_t mode = 0x03;
                return os_mbuf_append(ctxt->om, &mode, 1);
            }
            if (attr_handle == hid_max_count_handle) {
                static uint8_t max_touch_points = 0x05;
                return os_mbuf_append(ctxt->om, &max_touch_points, 1);
            }
            return 0;

        case 0x2A4E:
            static uint8_t p_mode = 0x01;
            return os_mbuf_append(ctxt->om, &p_mode, 1);
    }

    if (ctxt->dsc && ble_uuid_u16(ctxt->dsc->uuid) == 0x2908) {
        if (attr_handle == hid_tp_in_handle + 1)      return os_mbuf_append(ctxt->om, ref_tp_in, 2);
        if (attr_handle == hid_mouse_in_handle + 1)   return os_mbuf_append(ctxt->om, ref_mouse_in, 2);
        if (attr_handle == hid_ptphqa_handle + 1)     return os_mbuf_append(ctxt->om, ref_hqa, 2);
        if (attr_handle == hid_input_mode_handle + 1) return os_mbuf_append(ctxt->om, ref_mode, 2);
        if (attr_handle == hid_max_count_handle + 1) return os_mbuf_append(ctxt->om, ref_max_cnt, 2);
    }

    return 0;
}
const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        /*** HID Service: 0x1812 ***/
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x1812),
        .characteristics = (struct ble_gatt_chr_def[]) {
            
            /** HID Information: 0x2A4A **/
            {
                .uuid = BLE_UUID16_DECLARE(0x2A4A),
                .access_cb = gatt_svr_access_hid,
                .flags = BLE_GATT_CHR_F_READ,
            },

            /** Report Map: 0x2A4B **/
            {
                .uuid = BLE_UUID16_DECLARE(0x2A4B),
                .access_cb = gatt_svr_access_hid,
                .flags = BLE_GATT_CHR_F_READ,
            },

            /** Touchpad Input Report: 0x2A4D (ID 0x01) **/
            {
                .uuid = BLE_UUID16_DECLARE(0x2A4D), 
                .access_cb = gatt_svr_access_hid, 
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &hid_tp_in_handle, 
                .descriptors = (struct ble_gatt_dsc_def[]) {
                    { .uuid = BLE_UUID16_DECLARE(0x2908), .access_cb = gatt_svr_access_hid, .att_flags = BLE_ATT_F_READ }, 
                    { 0 }
                }
            },

            /** Max Count Feature Report: 0x2A4D (ID 0x03) **/
            {
                .uuid = BLE_UUID16_DECLARE(0x2A4D),
                .access_cb = gatt_svr_access_hid,
                .flags = BLE_GATT_CHR_F_READ,
                .val_handle = &hid_max_count_handle,
                .descriptors = (struct ble_gatt_dsc_def[]) {
                    { .uuid = BLE_UUID16_DECLARE(0x2908), .access_cb = gatt_svr_access_hid, .att_flags = BLE_ATT_F_READ },
                    { 0 }
                },
            },

            /** Mouse Input Report: 0x2A4D (ID 0x02) **/
            {
                .uuid = BLE_UUID16_DECLARE(0x2A4D),
                .access_cb = gatt_svr_access_hid,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &hid_mouse_in_handle,
                .descriptors = (struct ble_gatt_dsc_def[]) {
                    { .uuid = BLE_UUID16_DECLARE(0x2908), .access_cb = gatt_svr_access_hid, .att_flags = BLE_ATT_F_READ },
                    { 0 }
                },
            },

            /** PTPHQA Feature Report: 0x2A4D (ID 0x04) **/
            {
                .uuid = BLE_UUID16_DECLARE(0x2A4D),
                .access_cb = gatt_svr_access_hid,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                .val_handle = &hid_ptphqa_handle,
                .descriptors = (struct ble_gatt_dsc_def[]) {
                    { .uuid = BLE_UUID16_DECLARE(0x2908), .access_cb = gatt_svr_access_hid, .att_flags = BLE_ATT_F_READ },
                    { 0 }
                },
            },

            /** Input Mode Feature Report: 0x2A4D (ID 0x05) **/
            {
                .uuid = BLE_UUID16_DECLARE(0x2A4D),
                .access_cb = gatt_svr_access_hid,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                .val_handle = &hid_input_mode_handle,
                .descriptors = (struct ble_gatt_dsc_def[]) {
                    { .uuid = BLE_UUID16_DECLARE(0x2908), .access_cb = gatt_svr_access_hid, .att_flags = BLE_ATT_F_READ },
                    { 0 }
                },
            },

            /** Protocol Mode: 0x2A4E **/
            {
                .uuid = BLE_UUID16_DECLARE(0x2A4E),
                .access_cb = gatt_svr_access_hid,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE_NO_RSP,
            },

            /** HID Control Point: 0x2A4C **/
            {
                .uuid = BLE_UUID16_DECLARE(0x2A4C),
                .access_cb = gatt_svr_access_hid,
                .flags = BLE_GATT_CHR_F_WRITE_NO_RSP,
            },
            
            { 0 }
        },
    },

    /** Battery Service: 0x180F ***/

    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x180F),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID16_DECLARE(0x2A19),
                .access_cb = gatt_svr_access_hid,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
            },
            { 0 }
        },
    },
    { 0 }
};