#!/usr/bin/env bash

set -e

if [[ $1 == "clean" ]]; then
	rm build/* -rf
fi

gcc ./handmade.c -o build/handmade -lX11

if [[ $1 == "run" ]]; then
	build/handmade
fi
