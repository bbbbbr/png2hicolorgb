#!/bin/bash
set -euo pipefail

mkdir -p bin obj
# --type=1  : Median cut conversion with no dither
# --vaddrid : Map uses vram id instead of sequential tile order id
# -L=1 and -R=1 : Use adaptive mode for CGB palette block sizes (better image fidelity)
../bin/png2hicolorgb res/example_image.png --precompiled --type=1 -L=1 -R=1 -o obj/example_image.til --vaddrid
rgbasm -o obj/hicolor.o hicolor.asm
rgbasm -o obj/main.o main.asm
rgblink -m bin/hicolor.map -n bin/hicolor.sym -o bin/hicolor.gbc obj/hicolor.o obj/main.o
rgbfix -v -t hicolor -m 0x19 -r 0x00 -C bin/hicolor.gbc
