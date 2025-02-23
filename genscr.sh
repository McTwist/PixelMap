#!/bin/bash

if [[ $# -ne 3 ]]; then
	echo "Require pixelmapcli, world and out paths"
	exit 1
fi

PIXELMAPCLI="$1"
WORLD="$2"
OUT="$3"

# genimg(img, flags...)
genimg () {
	IMG=$1
	shift
	FILE="$OUT/$IMG.png"
	$PIXELMAPCLI "$WORLD" "$FILE" $@
	convert "$FILE" -trim +repage "$FILE"
}

mkdir -p "$OUT"
genimg "default"
genimg "night" "--night"
genimg "cave" "--cave"
genimg "gradient" "--gradient"
genimg "heightline" "--heightline" "4"
genimg "opaque" "--opaque"
genimg "slice" "--slice" "32"
genimg "gray" "-m" "gray"
genimg "color" "-m" "color"

