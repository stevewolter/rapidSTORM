#!/bin/sh

while read system distribution suffix archs; do \
    path="release/pbuilder/buildresult/$system/dists/$distribution/main"; \
    basename="rapidstorm_$suffix"; \
    for arch in $archs; do \
        echo "all : $path/${basename}_$arch.changes"; \
        echo "$path/${basename}_$arch.changes : DIST=$distribution"; \
        echo "$path/${basename}_$arch.changes : $basename.dsc"; \
    done; \
done << EOF
debian wheezy $1 i386 amd64 mingw-i686 mingw-amd64
ubuntu precise ${1}~ubuntu12.04.1 i386 amd64
ubuntu saucy $1 i386 amd64
EOF
