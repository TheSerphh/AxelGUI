cmake_minimum_required(VERSION 3.20)
project(AxelGui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "Clang is required to build this project.")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(axel-gui
    src/main.cpp
    src/ThemeManager.h
    src/ThemeManager.cpp
    src/DownloadTask.h
    src/DownloadTask.cpp
    src/MainWindow.h
    src/MainWindow.cpp
)

target_link_libraries(axel-gui PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# --- Standard Installation Rules for Linux / Arch ---
include(GNUInstallDirs)

# 1. Main binary -> /usr/bin/axel-gui
install(TARGETS axel-gui
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 2. Native messaging host script -> /usr/lib/axel-gui/axel-native-host
install(PROGRAMS native-host/axel-native-host
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/axel-gui
)

# 3. Firefox Native Messaging Manifest -> /usr/lib/mozilla/native-messaging-hosts/
install(FILES native-host/axel_gui_host.json
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/mozilla/native-messaging-hosts
)

# 4. Desktop Entry -> /usr/share/applications/
install(FILES axel-gui.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

# 5. Shared themes -> /usr/share/axel-gui/themes/
install(DIRECTORY themes/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/axel-gui/themes
)
