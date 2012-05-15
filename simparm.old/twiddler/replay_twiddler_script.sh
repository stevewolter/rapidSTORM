#!/bin/sh

sed -e 's/!at \([0-9]*\) \(.*\)/$current_time=\1; usleep (($current_time - $last_time)*1000); $last_time=$current_time; $| = 1; print "\2\\n"; /' \
    | { echo 'use Time::HiRes qw(usleep); $last_time = 0; '; cat -; }  \
    | perl
