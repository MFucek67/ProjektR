idf_component_register(
    SRCS "hal_mmwave_uart.c"
    INCLUDE_DIRS "include"
    REQUIRES driver platform mmwave board
)