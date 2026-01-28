idf_component_register(
    SRCS "./esp32/esp32_uart.c" "./esp32/esp32_time.c" "./esp32/esp32_task.c" "./esp32/esp32_queue.c" "./esp32/esp32_mutex.c" "./esp32/esp32_memory.c" "./esp32/esp32_events.c"
    INCLUDE_DIRS "include"
    REQUIRES board driver
)