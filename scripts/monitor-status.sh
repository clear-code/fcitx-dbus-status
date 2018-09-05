#!/bin/sh

dbus-monitor "type='signal',sender='org.fcitx.Fcitx',interface='org.fcitx.Fcitx.Status'"
