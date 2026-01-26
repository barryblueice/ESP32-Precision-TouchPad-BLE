idf_component_register(
    SRCS "main.c"
        "usb/usbhid.c"
        "usb/usb_descriptor.c"
        "ble/ble_descriptor.c"
        "ble/gatt_hid_report.c"
        "ble/gatt_init.c"
        "i2c/elan_i2c.c"
        "i2c/i2c_int.c"
        "nvs/ptp_nvs.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_driver_gpio
        esp_driver_i2c
        esp_tinyusb
        esp_timer
        nvs_flash
        esp_event
        esp_wifi
        bt
    )
