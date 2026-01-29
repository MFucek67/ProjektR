idf_component_register(
    SRCS "test_hal.c" "test_app.c" "test_mmwave_core.c"
    INCLUDE_DIRS "include"
    REQUIRES my_hal mmwave platform board app
)