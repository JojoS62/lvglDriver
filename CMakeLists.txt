add_library(lvglDriver INTERFACE)

target_include_directories(lvglDriver INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND SOURCEFILES 
    LVGLDispDriverBase.cpp
    LVGLDispDriver_GC9A01.cpp
    LVGLDispDriver_ST7735.cpp
    LVGLDispDriver_ILI9341.cpp
    LVGLInputDriverBase.cpp
    LVGLTouchDriverXPT2046.cpp
)

if("STM32F407VE_BLACK" IN_LIST MBED_TARGET_LABELS)
    list(APPEND SOURCEFILES
        TARGET_STM32F407VE_BLACK/ili9341_fsmc.cpp
        TARGET_STM32F407VE_BLACK/LVGLDispDriverSTM32F407VE_BLACK.cpp
        TARGET_STM32F407VE_BLACK/LVGLTouchDriver_STM32F407VE_BLACK.cpp
    )
endif()

if("DISCO_F769NI" IN_LIST MBED_TARGET_LABELS)
    target_link_libraries(lvglDriver INTERFACE bsp)
    list(APPEND SOURCEFILES
        TARGET_DISCO_F769NI/LVGLDispDriver_DISCO_F769NI.cpp
        TARGET_DISCO_F769NI/LVGLTouchDriverDISCO_F769NI.cpp
    )
endif()

if("DISCO_F746NG" IN_LIST MBED_TARGET_LABELS)
    target_link_libraries(lvglDriver INTERFACE bsp)
    list(APPEND SOURCEFILES
        TARGET_DISCO_F746NG/LVGLDispDriver_DISCO_F746NG.cpp
        TARGET_DISCO_F746NG/LVGLTouchDriverDISCO_F746NG.cpp
    )
endif()


if("FK743M5_XIH6" IN_LIST MBED_TARGET_LABELS)
    target_include_directories(lvglDriver INTERFACE "./TARGET_FK743")
    list(APPEND SOURCEFILES
        TARGET_FK743/lcd_fonts.cpp
        TARGET_FK743/display_ltdc.cpp
        TARGET_FK743/LVGLDispDriver_FK743.cpp
    )
endif()

target_sources(lvglDriver
    INTERFACE
        ${SOURCEFILES}
)

target_link_libraries(lvglDriver
    INTERFACE
        lvgl
)
