idf_component_register(
    SRCS
        main.cpp
        volume_knob.c
        my_display.c
    INCLUDE_DIRS .
    REQUIRES
        synth_core
        esp32-midi
        esp32-i2sbuf
        lvgl_esp32_drivers
        lvgl
)
