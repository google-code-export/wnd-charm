#!/usr/bin/perl

use strict;
use warnings;
use Getopt::Std;
use Image::Magick;

#################################################################################
#
#################################################################################
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

#################################################################################
#
#################################################################################
sub writeImage( \@ ) {

	my @marg_probs = $_[0];
	my $num_cols = $_[1];
	my $num_rows = $_[2];
	my $image = Image::Magick->new;
	my $res;
	
# This seems to be completely ignored.  The type is defined from the pixel values (!)
	$res = $image->Set(type=>'TrueColorMatte');
	warn "$res" if $res;
	$res = $image->Set(size=>"$width".'x'."$height");
	warn "$res" if $res;
	$res = $image->Set(depth=>8);
	warn "$res" if $res;
# These are conveniently ignored...
# use tiffcp -f msb2lsb for fill order
	$res = $image->Set(option=>'tiff:alpha=associated');
	warn "$res" if $res;
	$res = $image->Set(option=>'tiff:fill-order:msb2lsb');
	warn "$res" if $res;
	my @pixels;
	my ($row,$col);
# This mysterious call supersedes anything we do with setting the image type.
# It has to be transparent or the alpha channel is ignored
# Also, if we fail to do something like this, the image will be undefined after writing pixels to it.
# Of course, no error is reported while writing pixels - only when writing the file.
	$res = $image->Read ('CANVAS:transparent');
	warn "$res" if $res;

	my $r = 0, $g = 0, $b = 0, $a = 0;
	for( $row = 0; $row < $num_rows; $row++ ) {
		for( $col = 0; $col < $num_cols; $col++ ) {
		# For maximum efficiency, we can't set the RGB and Alpha in one call.
			$a = $marg_probs[$col][$row]->[0];
			@pixels = ( $a );
			$res = $image->SetPixel(channel => 'Alpha', 'x' => $x, 'y' => $y, 'color' => \@pixels);
			warn "$res" if $res;

			$r = $marg_probs[$col][$row]->[1];
			$g = $marg_probs[$col][$row]->[2];
			$b = $marg_probs[$col][$row]->[3];
			@pixels = ( $r, $g, $b );
			$res = $image->SetPixel(channel => 'RGB', 'x' => $x, 'y' => $y, 'color' => \@pixels);
			warn "$res" if $res;
		}
	}
	printf "\n";
	$res = $image->Write(filename=>'image.png');
	$res = $image->Write(filename=>'image.tiff');
	warn "$res" if $res;
}

#################################################################################
#
#################################################################################
sub OutputASCIIpic( \@ ) {
	my @marg_probs = $_[0];
	my $num_cols = $#marg_probs;
	my $num_rows = $#{ $marg_probs[0] };

	for( my $row = 0; $row <= $num_rows; $row++ ) {
		for( my $col = 0; $col <= $num_cols; $col++ ) {
			my $max = 0;
			my $index = 0
			my $index_of_highest = -1;
			foreach( @{ $marg_probs[$col][$row] } ) {
				if( $_ > $max ) {
					$max = $_;
					$index_of_highest = $index;
				}
				$index++;
			}
			if( $index_of_highest == 0 ) {
				print 'B';
			} elsif( $index_of_highest == 1 ) {
				print 'D';
			} elsif( $index_of_highest == 2 ) {
				print 'H';
			} elsif( $index_of_highest == 3 ) {
				print 'T';
			} else {
				print '?';
			}
		}
		print "\n";
	}
}

#################################################################################
# LoadInputFile( @matrix, $input_file )
#################################################################################
sub LoadInputFile( \@$ ) {
	my @results_matrix = $_[0];
	my $input_file = $_[1];

	open( IN, '<', $input_file ) or die "Couldn't open input file $input_file: $!\n";

	while( <IN> ) {
		if( /col (\d+), row (\d+): \((\d\.\d+)\s+(\d\.\d+)\s+(\d\.\d+)\s+(\d\.\d+)\)/ ) {
			print "loading col $1, row $2, marg probs( $3, $4, $5, $6 )\n";
			@{ $results_matrix[$1][$2] } = ( $3, $4, $5, $6 );
		}
	}
	close IN;
}

#################################################################################
# WriteOutputFile( @matrix, $output_file )
#################################################################################
sub WriteOutputFile( \@$ ) {
	my @results_matrix = $_[0];
	my $output_file = $_[1];

	my $num_cols = $#results_matrix;
	my $num_rows = $#{ $results_matrix[0] };

	open( OUT, '>', $output_file ) or die "Couldn't open output file $output_file: $!\n";

	for( my $col = 0; $col <= $num_cols; $col++ ) {
		for( my $row = 0; $row <= $num_rows; $row++ ) {
			print OUT "col $col, row $row: (";
			foreach( @{ $results_matrix[$col][$row] } ) {
				print OUT "$_\t";
			}
			print OUT ")\n";
		}
	}
	close OUT;
}

#################################################################################
# RunKernelScan( @matrix, $starting_fraction )
#################################################################################
sub RunKernelScan( \@$ ) {

	my @results_matrix = $_[0];
	my $starting_point = $_[1];
	my $image_width = 1392;
	my $image_height = 1040;
	my $kernel_width = 280;
	my $kernel_height = 280;
	my $granularity = 30;

	my $deltaX = int( $image_width / $granularity );
	my $deltaY = int( $image_height / $granularity );

	my $col = 0;
	my $row = 0;

	my $num_cols = int( ($image_width - $kernel_width) / $deltaX );
	my $num_rows = int( ($image_height - $kernel_height) / $deltaY );

	my $starting_col = int( $starting_point * $num_cols );

	my $x = 0;
	my $y = 0;

	for( my $col = $starting_col; $col <= $num_cols; $col++ ) {
		for( my $row = 0; $row <= $num_rows; $row++ ) {
			$x = $col * $deltaX;
			$y = $row * $deltaY;
			print "col $col, row $row, x: $x, y: $y, kernel width: $kernel_width, kernel height: $kernel_height\n";
			@{ $results_matrix[$col][$row] } = RunWNDCHRM_atROI( $x, $y, $kernel_width, $kernel_height );
		}
	}
}

#################################################################################
# main()
#################################################################################
sub main {

	my $starting_point = 0; # a number from 0 to 1 signifying percentage
	                        # where this instance should start calculating sigs.
	my $input_file = undef;
	my $output_file = undef;
	my %opts;
	my @results_matrix;

	if( getopts( 's:o:i:', \%opts ) ) {
		if( defined $opts{'s'} ) {
			print "found starting point: $opts{'s'}\n";
			$starting_point = $opts{'s'};
		}
		if( defined $opts{'o'} ) {
			print "marginal probs will be output to file $opts{'s'}\n";
			$output_file = $opts{'o'};
		}
		if( defined $opts{'i'} ) {
			print "marginal probs inputted from file $opts{'i'}\n";
			$input_file = $opts{'i'};
		}
	}

	if( defined $input_file ) {
		LoadInputFile( @results_matrix, $input_file );
	} else {
		RunKernelScan( @results_matrix, $starting_point );
	}

	if( defined $output_file ) {
		WriteOutputFile( @results_matrix, $output_file );
	}

	OutputASCIIpic( @results_matrix );

	writeImage( @results_matrix );
	return 0;
}

&main;
