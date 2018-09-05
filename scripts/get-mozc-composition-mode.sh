#!/bin/sh

dbus-send \
	--dest=org.fcitx.Fcitx \
	--type=method_call \
	--print-reply \
	/Status \
	org.fcitx.Fcitx.Status.Get \
	string:'mozc-composition-mode'
