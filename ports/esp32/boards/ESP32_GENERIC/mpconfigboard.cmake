include(boards/mpconfigboard_esp32_common.cmake)

list(APPEND SDKCONFIG_DEFAULTS
    boards/sdkconfig.csi)

set(SDKCONFIG_OPTS "${SDKCONFIG_OPTS} -DCONFIG_TWAI=y -DCONFIG_TWAI_ISR_IN_IRAM=y")
