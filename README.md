What's this?
====================
An add-on for [fcitx](https://gitlab.com/fcitx/fcitx) to set/get/monitor IM
statuses via D-Bus.

Basic Install
====================
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install

Please restart fcitx to enable it.

Dependency
=====================
fcitx (core and dbus module)
dbus

For example, you'll need to install these dependencies by the following command
to build fcitx-dbus-status on Debian/Ubuntu:

$ sudo apt-get install fcitx-libs-dev fcitx-module-dbus libdbus-1-dev

Usage
=====================
There are some examples under scripts directory:

  * get-mozc-composition-mode.sh
    * Get the compositon mode of fcitx-mozc
  * set-composition-mode.sh
    * Set the compositon mode of fcitx-mozc
  * monitor-status.sh
    * Monitor status changes of fcitx

Note: You can't on/off IM by this add-on. Please use "fcitx-remote" instead for
such purpose.
