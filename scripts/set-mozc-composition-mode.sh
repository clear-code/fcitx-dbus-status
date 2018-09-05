#!/bin/sh

print_usage() {
    echo "Usage: $0 {hiragana|katakana|half-katanaka|alnum|wide-alnum}"
    echo "Note: This script will works only when LANG=ja_JP.UTF-8"
}

case $1 in
    hiragana) MODE="ひらがな";;
    katakana) MODE="全角カタカナ";;
    wide-katakana) MODE="全角カタカナ";;
    half-katakana) MODE="半角カタカナ";;
    alnum) MODE="半角英数";;
    half-alnum) MODE="半角英数";;
    wide-alnum) MODE="全角英数";;
    *)  print_usage
        exit 1
        ;;
esac

dbus-send \
	--dest=org.fcitx.Fcitx \
	--type=method_call \
	--print-reply \
	/Status \
	org.fcitx.Fcitx.Status.Set \
	string:"mozc-composition-mode" \
	string:"${MODE}"
