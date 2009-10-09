#!/bin/sh

TEX=latex mpost logo.mp || exit 1
rm logo.mpx logo.log
convert -density 7200 logo.mps -resize x404 logo.png
montage logo.png 'AGLogo.png' -geometry x303 montage.png 
mv montage.png splash.png
