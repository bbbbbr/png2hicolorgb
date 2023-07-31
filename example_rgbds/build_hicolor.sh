#!/bin/bash

mkdir -p bin obj
../bin/png2hicolorgb res/example_image.png --type=1 -o=obj/example_image.til
rgbasm -o obj/hicolor.obj hicolor.s
rgblink -m obj/hicolor.map -n obj/hicolor.sym -o bin/hicolor.gbc obj/hicolor.obj
rgbfix -v -t hicolor -m 0x19 -r 0x00 -C bin/hicolor.gbc

