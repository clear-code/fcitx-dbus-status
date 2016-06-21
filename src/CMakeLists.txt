include_directories (${FCITX4_FCITX_INCLUDE_DIRS})
include_directories (${FCITX4_FCITX_CONFIG_INCLUDE_DIRS})
include_directories (${FCITX4_FCITX_UTILS_INCLUDE_DIRS})

set(FCITX_DBUS_STATUS_SOURCES
    dbus-status.cpp)

fcitx_add_addon_full(dbus-status
  SOURCES ${FCITX_DBUS_STATUS_SOURCES})
