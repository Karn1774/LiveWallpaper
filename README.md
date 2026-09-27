# Performance Live Wallpaper

A Plasma 6 wallpaper package that loops one local video file using Qt Multimedia.
The wallpaper is a standard Plasma KPackage; the small C++ library is a QML
extension module used to validate and normalize the selected local file URL.

## Build and install on Arch Linux

Install the development dependencies:

```sh
sudo pacman -S --needed base-devel cmake extra-cmake-modules libplasma qt6-base qt6-declarative qt6-multimedia
```

Build and install:

```sh
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build --parallel 2
sudo cmake --install build
```

Launch **Performance Live Wallpaper** from the application menu. Import one or
more MP4, WebM, MKV, or MOV files, select a video to preview it, set an optional
automatic change interval in minutes or hours, and select **Apply to all
screens**. Use **Pause wallpaper** to pause both playback and automatic changes,
**Resume wallpaper** to continue, or **Next wallpaper** to advance immediately.
Choose **In order** or **Random** playback; each switch fades through black
before the next video fades in. Automatic changes continue inside Plasma after the application closes. The
library remembers file paths but does not copy video data, so imported files
must remain at their original locations. This requires a running Plasma 6
session and an installed wallpaper package; Plasma may need to be restarted
once after installing a new package or changing its config schema so it reloads
the available playlist and scheduler settings.

The package is installed under `/usr/share/plasma/wallpapers/com.custom.livewallpaper/`.
Its QML extension is installed under the Qt 6 QML import directory, normally
`/usr/lib/qt6/qml/com/custom/livewallpaper/` on Arch. Plasma 6 wallpaper packages
are loaded as QML KPackages; `/usr/lib/qt6/plugins/plasma/wallpapers/` is not the
installation location for this architecture.

## Video performance

Qt Multimedia chooses its playback backend and decoder based on the installed Qt
build and graphics drivers. This project keeps one video player per wallpaper
instance and does not force a hardware-decoding mode, since unsupported forced
decoders can prevent playback. Hardware decoding is not guaranteed by Qt/QML
alone. Check that the video codec is supported by your system's Qt Multimedia
backend and that the Intel VA-API driver is available. Lower-resolution H.264
video at a modest frame rate is a practical fallback for older CPUs. Playback
uses no audio.

## Install location override

`LIVEWALLPAPER_QMLDIR` controls the native QML module path. On systems where Qt 6
uses a different import directory, configure with an explicit value, for example:

```sh
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr -DLIVEWALLPAPER_QMLDIR=/usr/lib/qt6/qml
```

## License

LiveWallpaper is open-source software released under the MIT License. See
[LICENSE](LICENSE) for the complete terms. Qt, KDE, and other dependencies
remain under their respective licenses.