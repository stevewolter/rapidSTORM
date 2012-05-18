while (<>) {
    $level += 3 if ( /^declare/ );
    $level -= 3 if ( /^end$/ );
    $indent = " "x$level;
    print $indent . $1 . "\n" if ( /^name (.*)$/ );
}
