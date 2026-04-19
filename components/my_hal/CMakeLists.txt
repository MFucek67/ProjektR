idf_component_register(
    SRCS "hal_network.c" "hal_wifi.c" "hal_ws.c" "hal_mmwave_uart.c" "system_monitor.c"
    INCLUDE_DIRS "include"
    REQUIRES driver platform mmwave board
)