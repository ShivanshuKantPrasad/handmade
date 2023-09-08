#!/usr/bin/env bash

set -e

if [[ $2 == "clean" ]]; then
	rm build/* -rf
fi

if [[ $1 == "linux" ]]; then

	gcc ./handmade.c -o build/handmade -lX11

	if [[ $2 == "run" ]]; then
		build/handmade
	fi

elif [[ $1 == "windows" ]]; then

	winegcc ./handmade_win.c -o build/handmade_win

	if [[ $2 == "run" ]]; then
		build/handmade_win.exe
	fi

fi
