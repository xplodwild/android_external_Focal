#!/usr/bin/perl

use strict;
use Getopt::Long;

my $createReference = 0;
my $verbose = 0;
GetOptions( '-c'     => \$createReference,
            '--verbose' => \$verbose,
    );

die "Too fee parameters" unless scalar(@ARGV) >= 2;

my $output = shift @ARGV;

# the rest of @ARGV is a list of files to use to create the PDD
my $tiff2psd = '../../tools/PTtiff2psd';

system ($tiff2psd, '-o', "$output" , @ARGV) == 0
    or die "executing of $tiff2psd failed: $?";

if ($createReference) {
    `mv -f $output reference/`;
} else {
    mkdir('tests');
    `mv -f $output tests/`;
    
    my $diff = `diff 'reference/$output' 'tests/$output' 2>&1`;
    print $diff;
    exit ($diff ne "");
}
