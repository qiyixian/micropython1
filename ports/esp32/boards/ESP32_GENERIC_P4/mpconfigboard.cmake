include(boards/mpconfigboard_esp32p4_common.cmake)
set(SDKCONFIG_OPTS "${SDKCONFIG_OPTS} -DCONFIG_TWAI=y -DCONFIG_TWAI_ISR_IN_IRAM=y")
