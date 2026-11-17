cmake_minimum_required(VERSION 3.16)

project(Container VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ✅ Include Qt6 Network and Xml components too
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Xml)

set(PROJECT_SOURCES
    main.cpp
    mainclient.cpp
    mainclient.h
    container.h container.cpp
    containerfactory.h containerfactory.cpp
    pallet.h pallet.cpp
    containerwidget.h containerwidget.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Container
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Icons.qrc
        serializing.h serializing.cpp




    )
else()
    if(ANDROID)
        add_library(Container SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(Container
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# ✅ Link with required Qt modules including Network and Xml
target_link_libraries(Container PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Xml
)

# Optional macOS/iOS bundle settings
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Container)
endif()

set_target_properties(Container PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Container
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Container)
endif()
