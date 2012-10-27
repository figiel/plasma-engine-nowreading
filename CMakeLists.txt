project(plasma-engine-nowreading)


# qt dependencies
#set(QT_USE_QTXMLPATTERNS TRUE)
#set(QT_USE_QTXML TRUE)
#find_package(Qt4 COMPONENTS QtCore QtXml QtXmlPatterns REQUIRED )
#include(${QT_USE_FILE})

# kde dependencies
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

find_package(PopplerQt4 REQUIRED)
include_directories(${KDE4_INCLUDES} ${POPPLER_QT4_INCLUDE_DIR})


# our files:
set(nowreading_SRCS nowreadingengine.cpp)

kde4_add_plugin(plasma_engine_nowreading ${nowreading_SRCS})
target_link_libraries(plasma_engine_nowreading
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${QT_LIBRARIES} ${POPPLER_QT4_LIBRARIES})

install(TARGETS plasma_engine_nowreading
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-engine-nowreading.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
