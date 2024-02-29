# Linux
Fuzzy Droplets requires gcc >= 11.4 and Qt >= 6.5. Many linux distributions will not have a version of Qt as recent as this. The easiest way of obtaining it is to use the Qt online installer. Below I will provide the steps to compile Fuzzy Droplets assuming you have a fresh install of Ubuntu (the steps will be very similar for other distributions).

* Upgrade the system
```
sudo apt update && sudo apt upgrade
```

* Install build toools including gcc, g++, cmake, ninja and git
```
sudo apt install build-essential cmake ninja-build git
```

* Install dependencies for QtCreator
```
sudo apt install libgl1-mesa-dev libvulkan-dev libxcb-xinput-dev libxcb-xinerama0-dev libxkbcommon-dev libxkbcommon-x11-dev libxcb-image0 libxcb-keysyms1 libxcb-render-util0 libxcb-xkb1 libxcb-randr0 libxcb-icccm4 libxcb-cursor0
```

* Download the [qt online installer](https://www.qt.io/download-qt-installer-oss)

* Open a terminal in your download folder and make the installer executable
```
chmod +x qt-unified-linux-x64-*
```

* Run the installer and install the latest desktop build system

* You will need to tell your system where to find the Qt binaries and libraries. We can use qtchooser ([manpage](https://manpages.ubuntu.com/manpages/jammy/man1/qtchooser.1.html)) to do this (also if we already have an older version of Qt installed)
```
sudo apt install qtchooser
```

* Add the recent installation of Qt to qtchooser's database (change ```mick``` to your username!)
```
qtchooser -install -f qt6 /home/mick/Qt/6.6.2/gcc_64/bin/qmake
```

* Add the following line to your ```.~profile``` to load your preferred Qt version at startup
```
export QT_SELECT=qt6
```
* Restart your system
* Run QtCreator, load the project by selecting ```CMakeLists.txt``` in the main folder of Fuzzy Droplets, and configure.
* Set build configuration to Release
* Build
