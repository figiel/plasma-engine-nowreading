project(plasma-engine-nowreading)

find_package(KDE4 REQUIRED)

include(KDE4Defaults)
include_directories(${KDE4_INCLUDES})


set(nowreading_SRCS nowreadingengine.cpp)

kde4_add_plugin(plasma_engine_nowreading ${nowreading_SRCS})

target_link_libraries(plasma_engine_nowreading
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_engine_nowreading
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-engine-nowreading.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
