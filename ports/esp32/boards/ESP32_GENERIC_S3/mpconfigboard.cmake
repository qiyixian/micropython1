include(boards/mpconfigboard_esp32s3_common.cmake)

list(APPEND SDKCONFIG_DEFAULTS
    boards/sdkconfig.flash_qio_80m
    boards/sdkconfig.csi
)
set(SDKCONFIG_OPTS "${SDKCONFIG_OPTS} -DCONFIG_TWAI=y -DCONFIG_TWAI_ISR_IN_IRAM=y")
