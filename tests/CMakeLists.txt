idf_component_register(
    SRCS "test_hal.c"
    INCLUDE_DIRS "include"
    REQUIRES my_hal mmwave platform board 
)