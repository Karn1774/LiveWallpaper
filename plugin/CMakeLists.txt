add_library(livewallpaperplugin SHARED
    ../src/wallpaperplugin.cpp
    ../src/wallpaperplugin.h
)

target_link_libraries(livewallpaperplugin PRIVATE
    Qt6::Core
    Qt6::Qml
)

install(TARGETS livewallpaperplugin
    LIBRARY DESTINATION ${LIVEWALLPAPER_QMLDIR}/com/custom/livewallpaper
)

install(FILES qmldir
    DESTINATION ${LIVEWALLPAPER_QMLDIR}/com/custom/livewallpaper
)