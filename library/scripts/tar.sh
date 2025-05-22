#!/bin/bash
# Unpack MC world, run it through cli and compare output

set -e

world="$1"
tar="$2"
render="$3"

# Temporary folder with trap
tmpdir=$(mktemp -d)
trap 'rm -rf -- "$tmpdir"' EXIT

# Needs to unpack them each time (cmake issues)
mkdir "$tmpdir/world"
tar -xf "$world" -C "$tmpdir/world" --strip-components=1
mkdir "$tmpdir/cmp"
tar -xf "$tar" -C "$tmpdir/cmp" --strip-components=1

# Run default and without output
$PIXELMAPCLI "$tmpdir/world" "$tmpdir/out" -q -r $render

# Compare output with prepared data
pushd "$tmpdir/cmp" > /dev/null
find . -type f -name "*.png" | xargs -I {} $COMPARE -metric AE "{}" "../out/{}" "$tmpdir/testimage.png"\;
popd > /dev/null
