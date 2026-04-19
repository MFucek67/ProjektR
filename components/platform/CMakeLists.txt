idf_component_register(
    SRCS "./esp32/esp32_uart.c" "./esp32/esp32_time.c" "./esp32/esp32_task.c" "./esp32/esp32_queue.c" "./esp32/esp32_mutex.c" "./esp32/esp32_memory.c" "./esp32/esp32_events.c"
        "./esp32/esp32_wifi_client.c" "./esp32/esp32_websocket.c"
    INCLUDE_DIRS "include"
    REQUIRES board driver my_hal esp_websocket_client esp_event nvs_flash esp_netif esp_wifi
)