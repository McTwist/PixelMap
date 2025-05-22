#!/bin/bash
# Unpack MC world, run it through cli and compare output

set -e

world="$1"
image="$2"
shift 2

# Temporary folder with trap
tmpdir=$(mktemp -d)
trap 'rm -rf -- "$tmpdir"' EXIT

# Needs to unpack them each time (cmake issues)
tar -xf "$world" -C "$tmpdir" --strip-components=1

# Run default and without output
$PIXELMAPCLI "$tmpdir" "$tmpdir/testimage.png" -q $@

# Compare output image with prepared image
$COMPARE -metric AE "$image" "$tmpdir/testimage.png" "$tmpdir/out.png"

