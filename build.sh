#!/usr/bin/env bash

set -e

if [[ $2 == "clean" ]]; then
	rm build/* -rf
fi

if [[ $1 == "linux" ]]; then

	gcc ./handmade.c -o build/handmade -lpulse -lpulse-simple -pthread -lm -lX11 -Wall

	if [[ $2 == "run" ]]; then
		build/handmade
	fi

elif [[ $1 == "windows" ]]; then

	winegcc ./handmade_win.cpp -o build/handmade_win -lgdi32 -Wall

	if [[ $2 == "run" ]]; then
		build/handmade_win.exe
	fi

elif [[ $1 == "wayland" ]]; then

	gcc xdg-shell-protocol.c handmade_wayland.cpp -o build/handmade_wayland -lwayland-client

	if [[ $2 == "run" ]]; then
		build/handmade_wayland
	fi
fi
