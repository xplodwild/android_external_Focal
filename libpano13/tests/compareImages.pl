#!/usr/bin/perl

use File::Temp qw/ tempfile tempdir /;
use strict;

use Getopt::Long;
my ($uncrop1, $uncrop2,$verbose);
GetOptions( 'uncrop1'     => \$uncrop1,
            'uncrop2'     => \$uncrop2,
            'verbose' => \$verbose,
    );

my $first = shift @ARGV;

my $second = shift @ARGV;

die "file not found [$first]" if (not -f $first);
die "file not found [$second]" if (not -f $second);


my ($fhTIFF, $tmp) = tempfile('tmpXXXX', {SUFFIX => 'tif', UNLINK => 1});
my ($fh, $tmp) = tempfile('tmpXXXX', {SUFFIX => 'png', UNLINK => 1});

my $output;
my $tmpFull1 = '';
my $tmpFull2 = '';


if ($uncrop1) {
    my $tmp = "tmp$$";
    $tmpFull1 = "tmp${$}0000.tif";
    `PTuncrop -p $tmp -q $first`;
    die "Unable to uncrop $first" if (not -f $tmpFull1);
    $first  = $tmpFull1;
} 

if ($uncrop2) {
    my $tmp = "tmpa${$}";
    $tmpFull2 = "tmpa${$}0000.tif";
    `PTuncrop -p $tmp -q $second`;
    die "Unable to uncrop $second" if (not -f $tmpFull2);
    $second  = $tmpFull2;
} 



$output = `compare -verbose -metric mae '$first' '$second' '$tmp' 2>&1`;
unlink    $tmpFull1 if $tmpFull1 ne "";
unlink    $tmpFull2 if $tmpFull2 ne "";


print $output;




