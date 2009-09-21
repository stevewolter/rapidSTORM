#!/bin/sh

echo "attach\ndetach\n" | ../src/dstorm --inputFile /srv/storm/K4\ Teil3.sif --OutputMethod Count --Twi > /dev/null
