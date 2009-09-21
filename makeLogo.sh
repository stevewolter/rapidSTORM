#!/bin/sh

TEX=latex mpost logo.mp || exit 1
rm logo.mpx logo.log
convert -density 7200 logo.mps -resize x404 logo.png
montage logo.png 'AGLogo.png' -geometry x303 montage.png 
convert montage.png -mattecolor white -frame 0x10 logo_mont.bmp
rm logo.mps logo.png
g++ convertToCHeader.cpp -lX11 -lpthread
./a.out logo_mont.bmp > logo.cpp
rm montage.png logo_mont.bmp a.out
