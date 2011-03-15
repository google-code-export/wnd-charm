#!/usr/bin/perl

use strict;
use warnings;
use Getopt::Std;

#wndchrm_debug classify  -l -B1112,760,280,280 NewNotRotated-ml.fit G\ -\ 9\(fld\ 37\ wv\ TL\ -\ DIC\ -\ Open\).tif

sub RunWNDCHRM_atROI($$$$)
{
	my $X = $_[0];
	my $Y = $_[1];
	my $deltaX = $_[2];
	my $deltaY = $_[3];

	my $wndchrm = "~/src/iicbu/wndchrm/branches/wndchrm-1.30/wndchrm";
	my $training_fit = "NewNotRotated-ml.fit";
	my $test_image = 'G\ -\ 9\(fld\ 37\ wv\ TL\ -\ DIC\ -\ Open\).tif';

	my $test_image_reg_exp = 'G - 9\(fld 37 wv TL - DIC - Open\)\.tif';

	my $cmd = "$wndchrm classify -l -s1 -B$X,$Y,$deltaX,$deltaY $training_fit $test_image";
	print "Running wndchrm command:\n $cmd \n";
	my @output = `$cmd`;

	#print "Here was the output: $output\n\n";
	foreach (@output) {
		if( /^$test_image_reg_exp\s+\S+\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/ ) {
			print "Caught marginal probabilities $1, $2, $3, $4\n";
			return ($1, $2, $3, $4 );
		}
	}

	return -1;
}

sub PrintLetter {
	my $aref = shift;
	my $max = 0;
	foreach (@$aref) {
		if( $_ > $max ) {
			$max = $_;
		}
	}

	if( $$aref[0] == $max ) {
		print 'B';
	} elsif( $$aref[1] == $max ) {
		print 'D';
	} elsif( $$aref[2] == $max ) {
		print 'H';
	} else {
		print 'T';
	}
}

sub main {

	my $image_width = 1392;
	my $image_height = 1040;
	my $kernel_width = 280;
	my $kernel_height = 280;
	my $granularity = 30;

	my $starting_point = 0; # a number from 0 to 1 signifying percentage
	                    # where this instance should start calculating sigs.
	my %opts;
	if( getopts( 's:', \%opts ) ) {
		print "found starting point: $opts{'s'}\n";
		$starting_point = $opts{'s'};
	} else {
		$starting_point = 0;
	}

	my $deltaX = int( $image_width / $granularity );
	my $deltaY = int( $image_height / $granularity );

	my $col = 0;
	my $row = 0;

	my $num_cols = int( ($image_width - $kernel_width) / $deltaX );
	my $num_rows = int( ($image_height - $kernel_height) / $deltaY );

	my $starting_col = int( $starting_point * $num_cols );

	my $x = 0;
	my $y = 0;
	my @results_matrix;

	for( my $col = $starting_col; $col <= $num_cols; $col++ ) {
		for( my $row = 0; $row <= $num_rows; $row++ ) {
			$x = $col * $deltaX;
			$y = $row * $deltaY;
			print "col $col, row $row, x: $x, y: $y, kernel width: $kernel_width, kernel height: $kernel_height\n";
			@{ $results_matrix[$col][$row] } = RunWNDCHRM_atROI( $x, $y, $kernel_width, $kernel_height );
		}
	}

	for( my $x = $starting_col; $x <= $#results_matrix; $x++ ) {
		for( my $y = 0; $y <= $#{ $results_matrix[0] }; $y++ ) {
			PrintLetter( \@{ $results_matrix[$x][$y] } );
		}
		print "\n";
	}
	return 0;
}

&main;
