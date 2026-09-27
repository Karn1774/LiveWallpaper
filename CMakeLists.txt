cmake_minimum_required(VERSION 3.16)

project(plasma-livewallpaper VERSION 1.0.0 LANGUAGES CXX)

find_package(ECM 1.4.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

set(QT_DEFAULT_MAJOR_VERSION 6)
set(LIVEWALLPAPER_QMLDIR "${CMAKE_INSTALL_LIBDIR}/qt6/qml" CACHE PATH "Qt 6 QML module installation directory")

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    Multimedia
    MultimediaWidgets
    Widgets
    DBus
)

find_package(Plasma CONFIG REQUIRED)

set(PLUGIN_ID "com.custom.livewallpaper")

plasma_install_package(package ${PLUGIN_ID} wallpapers wallpaper)

add_subdirectory(plugin)

add_executable(livewallpaper src/main.cpp)
target_link_libraries(livewallpaper PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::DBus
)

install(TARGETS livewallpaper
    RUNTIME DESTINATION ${KDE_INSTALL_BINDIR}
)

install(FILES data/com.custom.livewallpaper.desktop
    DESTINATION ${KDE_INSTALL_DATADIR}/applications
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)