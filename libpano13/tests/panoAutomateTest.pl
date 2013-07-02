#!/usr/bin/perl

use strict;

die "$0 [-c] <script> [test1 test2 ...]

-t       Only test output files (do not generate panoramas, useful to know which test failed
-c       Create reference files (should not be usually necessary)
<script> PTmender script
test_n   Test to run. If none given all are done

        jpeg, tiff, tifflze, tiffdeflate, tiff_m tiff_mask, psd, psd_nomask
\n " if scalar(@ARGV) < 1;


my $stitcher = '../../tools/PTmender';
#my $stitcher = '/usr/local/bin/PTmender';
$, = ' ';

my $arg = shift;


## make sure directories exist

mkdir('./tests');

my $createNewReferences = 0;
my $onlyTest = 0;

while (substr($arg,0,1) eq "-") {

  if ($arg eq "-c") {
    mkdir('./reference');
    $createNewReferences = 1;
  } elsif ($arg eq "-t") {
    $onlyTest = 1;
  }
$arg = shift;
}

open (IN, $arg) || die "Unable to open script $arg\n";


my $found = 0;

my $outputFormat;
my $pre;
my $post;
my $images = 0;

while (<IN>) {
    if (/^o/) {
	$images ++;
    }

    if (/^p/) {
	# this is the output line
	$outputFormat = $_;
	$found = 1;
    } else {
	if  (! $found) {
	    $pre .= $_;
	} else {
	    $post .= $_;
	}
    }
}

close IN;

my %formats = (
#    'jpeg' => "JPEG q100 g01", # jpeg progressive scan
#    'tiff_none' => "TIFF c:NONE",          # simple tiff output
#    'tiff_lzw' => "TIFF c:LZW",          # simple tiff output
#    'tiff_deflate' => "TIFF c:DEFLATE",          # simple tiff output
    'tiff_m' => "TIFF_m",        #  
#    'tiff_mask' => "TIFF_mask",
    'tiff_m_cropped'   => "TIFF_m r:CROP",
    'tiff_m_uncropped'   => "TIFF_m r:UNCROP",
#    'psd' => "PSD",
#    'psd_nomask' => "PSD_nomask",
#    'psd_mask' => "PSD_mask",

#    "xinvalid" => 'invalid',
);

my %outputFile = (
    'jpeg' => "output.jpg",
    'tiff_none' => "output.tif", 
    'tiff_lzw' => "output.tif",
    'tiff_deflate' => "output.tif",   
    'psd' => "output.psd",
    'psd_nomask' => "output.psd",
    'psd_mask' => "output.psd",
    );

my %savedFile = (
    "jpeg"        => "jpeg.jpg",
    "psd_nomask"  => "psd_no_mask.psd",
    'psd_mask' => "psd_mask.psd",
    "psd"         => "psd.psd",
    "tiff_none"    => "tiff_none.tif",
    "tiff_lzw"     => "tiff_lzw.tif",
    "tiff_deflate" => "tiff_deflate.tif",
    );


my $type;

if ($outputFormat =~ /n"/) { #"
    $outputFormat = $` ; #`
}


print "Processing $images images\n";

# WHat types of images do we generate? by default all, otherwise use command line  parameters

my @toProcess;

if (scalar(@ARGV) > 0 ) {
    @toProcess = @ARGV;
    print "To perform tests: ", @toProcess , "\n\n";
} else {
    @toProcess = sort keys (%formats);
    print "To perform all tests: ", @toProcess, "\n\n";
}

# Where do we put the output files? 
my $destination; 

if ($createNewReferences) {
    $destination = 'reference';
} else {
    $destination = 'tests';
}

my $testsPassed = 0;
my $testsCount = 0;

foreach $type (@toProcess) {
    $testsCount ++;

    
    defined $formats{$type} || die "$type is not a  defined test";


    print "$type => Output : $formats{$type}\n";
    
    if (! $onlyTest) {
      Create_Script($type, $pre, $post, $outputFormat);
      
      Remove_Images("output", "tif", $images, $destination, $type);
      
      print "Creating panorama.. please wait\n";
      system ($stitcher, '-o', 'output' , 'temp.txt');
      
      
      Move_Images("output", "tif", $images, $destination, $type);
      
    }
    if (! $createNewReferences) {
	
	my $output = "";

        $output = Compare_Images("tif", $images, "tests", "reference", $type);
	
	if ($output eq "") {
	  printf "\nTest $type................................. passed\n\n";
	  $testsPassed ++;
	} else {
	  printf "\nTest $type................................. FAILED!!!\n\n";
	}
    }
}

if (!$createNewReferences) {
  my $testsFailed = $testsCount - $testsPassed;
  printf "\nSummary: $testsCount tests executed.  $testsPassed passed ($testsFailed failed) %5.2f percent passed\n", $testsPassed * 100 /$testsCount;
  if ($testsPassed != $testsCount ) {
    die "BAD NEWS\n";
  }
} else {
  printf "\nSummary: $testsCount reference tests executed.\n";
}


sub Compare_Images
{
    my ($extension, $count, $newVersion, $reference, $prefixOutput) =@_;
    my $i= 0;
    
    my $output;

    print "Comparing reference images: $count\n";

    for ($i=0; $i < $count; $i++ ) {
	my $newFileName = sprintf("$newVersion/${prefixOutput}%04d.", $i) . $extension;
	my $fileName = sprintf("$reference/${prefixOutput}%04d.", $i) . $extension;
	print "$fileName -> $newFileName\n";
	
	if (! -f "$newFileName") {
	    printf("Output file $newFileName was not created\n");
	    return "Output file $newFileName was not created\n";
	}
	if (! -f "$fileName") {
	    printf("Reference file $fileName does not exist\n");
	    return "Reference file $fileName does not exist\n";
	}


	if ($fileName =~ /\.tif$/) {
	    $output .= `tiffcmp $fileName  $newFileName | grep -v 'Created by Panotools'`;
	} else {
	    $output .= `diff $fileName  $newFileName`;
	}
    }
    return $output;
}


sub Move_Images
{
    my ($prefix, $extension, $count, $destination, $prefixOutput) =@_;
    my $i= 0;
    
    print "Moving reference images: $count\n";

    for ($i=0; $i < $count; $i++ ) {
	my $fileName = sprintf("${prefix}%04d.", $i) . $extension;
	my $newFileName = sprintf("$destination/${prefixOutput}%04d.", $i) . $extension;
	print "$fileName -> $newFileName\n";
	`mv -f $fileName  $newFileName`;
    }
}

sub Remove_Images
{
    my ($prefix, $extension, $count, $destination, $prefixOutput) =@_;
    my $i= 0;
    
    print "Removing old images: $count\n";

    for ($i=0; $i < $count; $i++ ) {
	my $fileName = sprintf("${prefix}%04d.", $i) . $extension;
	my $newFileName = sprintf("$destination/${prefixOutput}%04d.", $i) . $extension;
	print "$fileName -> $newFileName\n";
	`rm -f $newFileName`;
    }
}




sub Create_Script
{
    my ($type, $pre, $post, $outputFormat) = @_;
    open OUT, ">temp.txt" || die "unable to create output script temp.txt\n";
    
    print OUT $pre;
    print OUT "$outputFormat n\"$formats{$type}\"\n";
    
    print OUT $post;
    
    close OUT;
}


sub execute
{
    my ($c) = @_;
    `$c`;
    my $status = ($? >> 8);
    die "execution of program [$c] failed: status [$status]" if ($status != 0);
}
