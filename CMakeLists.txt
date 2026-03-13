cmake_minimum_required(VERSION 3.16)
project(DentaPro VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core Widgets Sql PrintSupport
)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/database.cpp
    src/patientdialog.cpp
    src/interventiondialog.cpp
    src/patienthistorywidget.cpp
    src/signaturewidget.cpp
    src/reportgenerator.cpp
    src/clinicsettings.cpp
    src/clinicsettingsdialog.cpp
    src/statisticswidget.cpp
    src/exportmanager.cpp
)

set(HEADERS
    src/mainwindow.h
    src/database.h
    src/models.h
    src/patientdialog.h
    src/interventiondialog.h
    src/patienthistorywidget.h
    src/signaturewidget.h
    src/reportgenerator.h
    src/clinicsettings.h
    src/clinicsettingsdialog.h
    src/statisticswidget.h
    src/exportmanager.h
)

set(RESOURCES
    resources/resources.qrc
)

qt_add_executable(DentaPro
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_link_libraries(DentaPro PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
)

set_target_properties(DentaPro PROPERTIES
    WIN32_EXECUTABLE TRUE
)
