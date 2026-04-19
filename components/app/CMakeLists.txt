idf_component_register(
    SRCS "app_network_packet_serializer.c" "app_network.c" "app_mmwave_decoder.c" "app_mmwave_hal_config.c" "app_mmwave.c" "mmWave_manager.c"
    INCLUDE_DIRS "include"
    REQUIRES my_hal mmwave platform
    PRIV_REQUIRES board
)