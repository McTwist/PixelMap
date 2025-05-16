#!/bin/bash
# Unpack MC world, run it through cli and compare output

set -e

WORLD="$1"
IMAGE="$2"

# Temporary folder with trap
tmpdir=$(mktemp -d)
trap 'rm -rf -- "$tmpdir"' EXIT

# Needs to unpack them each time (cmake issues)
tar -xf "$WORLD" -C "$tmpdir" --strip-components=1

echo $PIXELMAPCLI
# Run default and without output
$PIXELMAPCLI "$tmpdir" "$tmpdir/testimage.png" -q

# Compare output image with prepared image
$COMPARE -metric AE "$IMAGE" "$tmpdir/testimage.png" "$tmpdir/out.png"

