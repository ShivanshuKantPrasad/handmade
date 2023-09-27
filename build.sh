#!/usr/bin/env bash

set -e

if [[ $2 == "clean" ]]; then
	rm build/* -rf
fi

if [[ $1 == "linux" ]]; then

	gcc ./handmade.c -o build/handmade -lpulse-simple -pthread -lm -lX11 -Wall

	if [[ $2 == "run" ]]; then
		build/handmade
	fi

elif [[ $1 == "windows" ]]; then

	winegcc ./handmade_win.cpp -o build/handmade_win -lgdi32 -Wall

	if [[ $2 == "run" ]]; then
		build/handmade_win.exe
	fi

fi
