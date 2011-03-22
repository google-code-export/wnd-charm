#!/usr/bin/perl


use strict;
use warnings;
use Getopt::Std;
use Image::Magick;

#################################################################################
# #FIXME: The pixel values are hard coded for a 4 class problem
#################################################################################
sub writeImage {

	my $marg_probs_ref = shift;
	my $output_image_path = shift;
	my $num_cols = $#{ $marg_probs_ref };
	my $num_rows = $#{ $marg_probs_ref->[0] };
	my $image = Image::Magick->new;
	my $res;
	
# This seems to be completely ignored.  The type is defined from the pixel values (!)
	$res = $image->Set(type=>'TrueColorMatte');
	warn "$res" if $res;
	$res = $image->Set(size=>"$num_cols".'x'."$num_rows");
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

	my $r = 0;
	my $g = 0;
	my $b = 0;
	my $a = 0;
	for( $row = 0; $row < $num_rows; $row++ ) {
		for( $col = 0; $col < $num_cols; $col++ ) {
		# For maximum efficiency, we can't set the RGB and Alpha in one call.
			$a = $$marg_probs_ref[$col][$row]->[0];
			@pixels = ( $a );
			$res = $image->SetPixel(channel => 'Alpha', 'x' => $col, 'y' => $row, 'color' => \@pixels);
			warn "$res" if $res;

			$r = $$marg_probs_ref[$col][$row]->[1];
			$g = $$marg_probs_ref[$col][$row]->[2];
			$b = $$marg_probs_ref[$col][$row]->[3];
			@pixels = ( $r, $g, $b );
			$res = $image->SetPixel(channel => 'RGB', 'x' => $col, 'y' => $row, 'color' => \@pixels);
			warn "$res" if $res;
		}
	}
	printf "\n";
	$res = $image->Write(filename=>$output_image_path);
	warn "$res" if $res;
}

#################################################################################
# #FIXME: The ASCII output is hardcoded for a 4 class problem,
#         with a specific order of specific classes in the marginal probabilities
#################################################################################
sub OutputASCIIpic {
	my $marg_probs_ref = shift;
	my $num_cols = $#{ $marg_probs_ref };
	my $num_rows = $#{ $marg_probs_ref->[0] };

	for( my $row = 0; $row <= $num_rows; $row++ ) {
		for( my $col = 0; $col <= $num_cols; $col++ ) {
			my $max = 0;
			my $index = 0;
			my $index_of_highest = -1;
			foreach( @{ $$marg_probs_ref[$col][$row] } ) {
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
# LoadInputFile()
#       inputs: $input_file
#       output: @results_matrix
# #FIXME: This is also a 4-class application specific function
#################################################################################
sub LoadInputFile {
	my $input_file = shift;
	my @results_matrix;

	open( IN, '<', $input_file ) or die "Couldn't open input file $input_file: $!\n";

	# ex: "col 7, row 4: (0.000	0.810	0.188	0.003	)"

	while( <IN> ) {
		if( /col (\d+), row (\d+): \((\d\.\d+)\s+(\d\.\d+)\s+(\d\.\d+)\s+(\d\.\d+)/ ) {
			print "loading col $1, row $2, marg probs( $3, $4, $5, $6 )\n";
			@{ $results_matrix[$1][$2] } = ( $3, $4, $5, $6 );
		}
	}
	close IN;
	return @results_matrix;
}

#################################################################################
# WriteOutputFile( \@matrix, $output_file )
#################################################################################
sub WriteOutputFile{
	my ($results_matrix_ref, $output_file) = @_;

	my $num_cols = $#{ $results_matrix_ref };
	my $num_rows = $#{ $results_matrix_ref->[0] };

	print "Writing output file: $output_file with $num_cols columns and $num_rows rows.\n";

	open( OUT, '>', $output_file ) or die "Couldn't open output file $output_file: $!\n";

	for( my $col = 0; $col <= $num_cols; $col++ ) {
		for( my $row = 0; $row <= $num_rows; $row++ ) {
			print OUT "col $col, row $row: (";
			foreach( @{ $$results_matrix_ref[$col][$row] } ) {
				print OUT "$_\t";
			}
			print OUT ")\n";
		}
	}
	close OUT;
}

#################################################################################
# RunKernelScan()
#   required inputs: $path_to_wndchrm, $image_name, $training_set_fit_file, $deltaX, $deltaY
#   optional inputs: $starting_fraction
#   output:          @results_matrix
#################################################################################
sub RunKernelScan( $$$$$;$$ ) {

	my( $path_to_wndchrm, $test_image, $training_fit, $deltaX, $deltaY, $starting_point, $wndchrm_args )= @_;

	if( !defined $wndchrm_args ) { $wndchrm_args = ""; }

	my $image_width = 1392; #FIXME: use tiffinfo to automatically extract image pixel dimensions
	my $image_height = 1040;
	my $kernel_width = 280; #FIXME: this should be a command line input
	my $kernel_height = 280;

	my @results_matrix;

  my $test_image_shell = $test_image;
	my $test_image_reg_exp = $test_image;

	$test_image_shell =~ s/([ \(\)])/\\$1/g; # for shell, spaces and parentheses need to be escaped
	$test_image_reg_exp =~ s/([\(\)\.])/\\$1/g; # for regular expressions, parens and periods need to be escaped

	my $retval = 0;

	my $num_cols = int( ($image_width - $kernel_width) / $deltaX );
	my $num_rows = int( ($image_height - $kernel_height) / $deltaY );

	my $starting_col = int( $starting_point * $num_cols );
	my $remainder = ( $starting_point * $num_cols ) - $starting_col;
	my $starting_row = int( $remainder * $num_rows );

	print "Image will be scanned with $num_cols columns and $num_rows rows, starting at column $starting_col\n";
	my $x = 0;
	my $y = 0;

	my $first_time_through = 1;
	for( my $col = $starting_col; $col <= $num_cols; $col++ ) {
		for( my $row = 0; $row <= $num_rows; $row++ ) {
			if( $first_time_through ) {
				$row = $starting_row;
				$first_time_through = 0;
			}
			$x = $col * $deltaX;
			$y = $row * $deltaY;
			print "col $col, row $row, x: $x, y: $y, kernel width: $kernel_width, kernel height: $kernel_height\n";

			my $cmd = "$path_to_wndchrm classify $wndchrm_args -s1 -B$x,$y,$kernel_width,$kernel_height $training_fit $test_image_shell 2>&1";
			print "Running wndchrm command:\n $cmd \n";
			my @output = `$cmd`;
			$retval = $? >> 8;

			if( $retval != 1 ) {
				print "WNDCHRM returned error: $retval\n";
				print "Wndchrm output:\n";
				foreach (@output) { print "$_\n"; }
				print "\n";
				die;
			}
			#print "Here was the output: $output\n\n";
			foreach (@output) {
				if( /^$test_image_reg_exp\s+\S+\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/ ) {
					print "Caught marginal probabilities $1, $2, $3, $4\n";
					@{ $results_matrix[$col][$row] } = ($1, $2, $3, $4 );
				}
			}
		}
	}
	return @results_matrix;
}

#################################################################################
#
#################################################################################
sub ShowHelp {
	print "WNDCHRM heatmap generator, ver 1.30\n";
	print "\n";
	print "There are two options to specify input for heatmap generator:\n";
	print "Method 1: Perform a fresh window scan by specifying the -i, -w, -t, and -p parameters.\n";
	print "\t-i <path>  : Input image: the image which will be scanned.\n";
	print "\t-w <path>  : WNDCHRM path: path to the wndchrm executable.\n";
	print "\t-t <path>  : Training Set: path to the .fit file containing the classifier.\n";
	print "\t             must be generated beforehand usind WNDCHRM\n";
	print "\t-p <#x#>   : Set displacement of scanning window to move NxN pixels. \n";
	print "\te.g., heatmap.pl -i input_image.tiff -w /path/to/wndchrm -t /path/to/training_set.fit -p 20x20\n";
	print "\n";
	print "Method 2: Create heatmap using a marginal probability dumpfile generated by a previous scan.\n";
	print "\t-l <path>  : Load a file containing dumped marginal probabilities created by previously running this script\n";
	print "\t             using the -d option.\n";
	print "\te.g., heatmap.pl -l dumpfile.txt\n";
	print "\n";
	print "additional optional arguments:\n";
	print "\t-s <#>     : Starting point: where # is a decimal between 0.0 and 1.0 indicating where this instance of\n";
	print "\t             the script should start calculating features for given input image\n";
	print "\t-d <path>  : Dump marginal probabilities to this file to save time when re-running\n";
	print "\t             this script later.\n";
	print "\t-o <path>  : Specify a filename and path to the generate heatmap image.\n";
	print "\t           : otherwise, image is created with deafult name \"image.tif\"\n";
	print "\t-a \"string\"  : Specify wndchrm command line arguments, e.g., -a \"-l -f0.03\"\n";
}

#################################################################################
# main()
#################################################################################
sub main {

	my $starting_point = 0; # a number from 0 to 1 signifying percentage
	                        # where this instance should start calculating sigs.
	my $input_file = undef;
	my $output_file = undef;
	my $path_to_wndchrm = undef;
	my $input_image = undef;
	my $training_fit = undef;
	my $heatmap_image_path = undef;
	my $heatmap_resolution_string = undef;
	my $wndchrm_args = undef;
	my( $deltaX, $deltaY );
	my %opts;
	my @results_matrix;

	if( getopts( 'w:i:t:p:;s:d:l:o:a:', \%opts ) ) {
		$path_to_wndchrm = $opts{'w'};
		$input_image = $opts{'i'};
		$training_fit = $opts{'t'};
		$heatmap_resolution_string = $opts{'p'};
		$starting_point = $opts{'s'};
		$output_file = $opts{'d'};
		$input_file = $opts{'l'};
		$heatmap_image_path = $opts{'o'};
		$wndchrm_args = $opts{'a'};
	} else {
		&ShowHelp;
		return -1;
	}

  # you need to have an input. there are two ways to specify inputs:
	# the four parameters to run the kernel scan, or the one parameter to use the
	# dumpfile generated by a previous scan.
	if( !( $path_to_wndchrm && $input_image && $training_fit && $heatmap_resolution_string ) ) {
		if( !$input_file ) {
			&ShowHelp;
			return -1;
		}
	}
	else { # we have all four parameters required for fresh kernel scan
		if( $heatmap_resolution_string =~ /(\d+)x(\d+)/ ) {
			$deltaX = $1;
			$deltaY = $2;
		} else {
			print "Incorrectly formatted pixel dimensions ($heatmap_resolution_string)\n";
			print "Please enxer pixel dimension in the form of #x#, for example, 20x20\n";
			return -2;
		}
	}

	if( $path_to_wndchrm ) {
		print "path to wndchrm: $path_to_wndchrm\n";
	}
	if( $input_image ) {
		print "input image: $input_image\n";
	}
	if( $training_fit ) {
		print "training set: $training_fit\n";
	}
	if( $heatmap_resolution_string ) {
		print "heatmap resolution: $heatmap_resolution_string\n";
	}
	if( $starting_point ) {
		print "found starting point: $starting_point\n";
	}
	if( $output_file ) {
		print "marginal probs will be output to file $output_file\n";
	}
	if( $input_file ) {
		print "marginal probs inputted from file $input_file\n";
	}
	if( $wndchrm_args ) {
		print "wndchrm arguments: $wndchrm_args\n";
	}

	if( $heatmap_image_path && $heatmap_image_path ne "" ) {
		if( $heatmap_image_path =~ /tif$|tiff$|png$/ ) {
			print "Output heatmap image will be saved to file $heatmap_image_path\n";
		}
		else {
			warn "Filename $heatmap_image_path is invalid: Must end with a .tif, .tiff, or .png extension. Using default name \"image.tif\"\n";
			$heatmap_image_path = "image.tif";
		}
	} else {
		print "Output heatmap image will be saved to default file name ./image.tif\n";
		$heatmap_image_path = "image.tif";
	}

	if( defined $input_file ) {
		@results_matrix = LoadInputFile( $input_file );
	} else {
		@results_matrix = RunKernelScan( $path_to_wndchrm, $input_image, $training_fit, $deltaX, $deltaY, $starting_point, $wndchrm_args);
	}

	print "Loaded results matrix with $#results_matrix columns and $#{ $results_matrix[0] } rows.\n";

	if( defined $output_file ) {
		WriteOutputFile( \@results_matrix, $output_file );
	}

	OutputASCIIpic( \@results_matrix );

	writeImage( \@results_matrix, $heatmap_image_path );
	return 1;
}

&main;
