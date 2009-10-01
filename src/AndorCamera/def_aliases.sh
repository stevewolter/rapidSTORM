#!/bin/sh

csplit $1 '/^[    ]*EXPORTS[      ]*$/+1' >/dev/null
sed -n -e 's/.* undefined reference to `__imp__\(.*\)\(@[0-9]*\).*/_imp__\1\2 = \1/p' | cat xx00 - xx01
