while (<STDIN>) {
    print;
    if ( /^Checksums-Sha1:/ ) {
        for my $file (@ARGV) {
            my $sha1sum = `sha1sum $file`;
            $sha1sum =~ s/ .*//;
            chomp($sha1sum);
            my $size = -s $file;
            print " $sha1sum " . $size . " " . $file . "\n";
        }
    }
    if ( /^Checksums-Sha256:/ ) {
        for my $file (@ARGV) {
            my $check = `sha256sum $file`;
            $check =~ s/ .*//;
            chomp($check);
            my $size = -s $file;
            print " $check " . $size . " " . $file . "\n";
        }
    }
    if ( /^Files:/ ) {
        for my $file (@ARGV) {
            my $check = `md5sum $file`;
            $check =~ s/ .*//;
            chomp($check);
            my $size = -s $file;
            open DPKG, "-|", "dpkg", "-I", $file;
            my $section;
            my $priority;
            while (<DPKG>) {
                $priority = $1 if ( /^ Priority: (.*)/ );
                $section = $1 if ( /^ Section: (.*)/ );
            }
            close DPKG;
            print " $check $size $section $priority $file\n";
        }
    }
}
