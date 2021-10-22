add_library(lvglDriver STATIC)

target_include_directories(lvglDriver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(lvglDriver
    PRIVATE
        LVGLDispDriverBase.cpp
        LVGLDispDriver_GC9A01.cpp
        LVGLDispDriver_ST7735.cpp
        LVGLInputDriverBase.cpp
        LVGLTouchDriverXPT2046.cpp
        TARGET_STM32F407VE_BLACK/ili9341_fsmc.cpp
        TARGET_STM32F407VE_BLACK/LVGLDispDriverSTM32F407VE_BLACK.cpp
        TARGET_STM32F407VE_BLACK/LVGLTouchDriver_STM32F407VE_BLACK.cpp
)

target_link_libraries(lvglDriver
    PRIVATE
        lvgl
        mbed-os
)
