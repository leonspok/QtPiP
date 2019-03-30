#!/bin/bash

# Requires xclip to be installed
selectedText=$(xclip -out -selection primary)
#notify-send "Selected text" "Playing: $selectedText"
qtpip -s "$selectedText" $@
