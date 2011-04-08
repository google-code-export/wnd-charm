#!/usr/bin/perl


use strict;
use warnings;
use Image::Magick;
use Getopt::Long qw(:config auto_abbrev no_ignore_case);

# Forward Declarations
sub writeImage;
sub OutputASCIIpic;
sub LoadInputFile;
sub WriteOutputFile;
sub RunWNDCHARM_atROI;
sub FullKernelScan;
sub LinearKernelScan;
sub PrintLinearKernelScan;
sub ShowHelp;
sub main;

&main;

#################################################################################
# main()
#################################################################################
sub main {

	my $starting_percentage = 0; # a number from 0 to 1 signifying percentage
	                        # where this instance should start calculating sigs.
	my $input_file = undef;
	my $output_file = undef;
	my $path_to_wndchrm = undef;
	my $input_image = undef;
	my $training_fit = undef;
	my $heatmap_image_path = undef;
	my $kernel_displacement = undef;
	my $wndchrm_args = undef;
	my $quiet = undef;
	my $start_coords = undef;
	my $end_coords = undef;
	my $granularity = undef;
	my $kernel_size = undef;
	my $kernel_width = undef;
	my $kernel_height = undef;
	my $deltaX = undef;
	my $deltaY = undef;
	my( $x1, $y1, $x2, $y2 );
	my %opts;
	my @results_matrix;

	GetOptions(
		"wndchrm=s" => \$path_to_wndchrm,
		"input_image=s" => \$input_image,
		"training_fit=s" => \$training_fit,
		"kernel_size=s"=> \$kernel_size,
		"placement=s" => \$kernel_displacement,
		"starting_percentage=f" => \$starting_percentage,
		"dump_marg_probs_to_file=s" => \$output_file,
		"load_marg_probs_from_file=s" => \$input_file,
		"output_heatmap=s" => \$heatmap_image_path,
		"args=s" => \$wndchrm_args,
		"quiet" => \$quiet, 
		"begin_coords=s" => \$start_coords,
		"granularity=i" => \$granularity,
		"end_coords=s" => \$end_coords,
		"help|?" => sub { &ShowHelp; return -1; }
		);

	# Figure out what the user wants
	my $full_scan = 0;
	my $start_end_line_scan = 0;
	my $start_displacement_line_scan = 0;
	if( $path_to_wndchrm && $input_image && $training_fit && $kernel_size ) {
		if( $start_coords && $end_coords && $granularity) {
			$start_end_line_scan = 1;
		}
		elsif( $start_coords && $kernel_displacement ) {
			$start_displacement_line_scan = 1;
		}
		elsif( $kernel_displacement ) {
			$full_scan = 1;
		}
	}
	if( !$input_file && !$start_end_line_scan && !$start_displacement_line_scan
		  && !$full_scan ) {
		warn "Insufficient arguments\n";
		&ShowHelp;
		return -3;
	}

	if( $kernel_size =~ /(\d+)x(\d+)/) {
		$kernel_width = $1;
		$kernel_height = $2;
		print "Kernel width: $kernel_width, height $kernel_height\n" if !$quiet;
	} else {
		warn "Incorrectly formatted pixel dimensions ($kernel_displacement)\n";
		warn "Please enxer pixel dimension in the form of #x#, for example, 20x20\n";
		return -2;
	}
	if( $kernel_displacement ) {
		if( $kernel_displacement =~ /(-?\d+)x(-?\d+)/ ) {
			$deltaX = $1;
			$deltaY = $2;
			print "Window displacement: $deltaX px in x-dir, & $deltaY px in y-dir.\n" if !$quiet;
		} else {
			warn "Incorrectly formatted pixel dimensions ($kernel_displacement)\n";
			warn "Please enxer pixel dimension in the form of #x#, for example, 20x20\n";
			return -2;
		}
	}
	if( $start_coords ) {
		if( $start_coords =~ /(\d+),(\d+)/ ) {
			$x1 = $1;
			$y1 = $2;
			print "Begin coords for linear scan: x: $x1, y: $y1\n" if !$quiet;
		} else {
			warn "Incorrectly formatted start coordinates ($start_coords)\n";
			warn "Please enter pixel coords in the form of #,#, for example, 20,20\n";
			return -2;
		}
	}
	if( $end_coords ) {
		if( $end_coords =~ /(\d+),(\d+)/ ) {
			$y2 = $2;
			print "Begin coords for linear scan: x: $x2, y: $y2\n" if !$quiet;
		} else {
			warn "Incorrectly formatted end coordinates ($end_coords)\n";
			warn "Please enter pixel coords in the form of #,#, for example, 20,20\n";
			return -2;
		}
	}

	if( !$quiet && $path_to_wndchrm ) {
		print "path to wndchrm: $path_to_wndchrm\n";
	}
	if( !$quiet && $input_image ) {
		print "input image: $input_image\n";
	}
	if( !$quiet && $training_fit ) {
		print "training set: $training_fit\n";
	}
	if( !$quiet && $kernel_displacement ) {
		print "kernel displacement: $kernel_displacement\n";
	}
	if( !$quiet && $starting_percentage ) {
		print "found starting point: $starting_percentage\n";
	}
	if( !$quiet && $output_file ) {
		print "marginal probs will be output to file $output_file\n";
	}
	if( !$quiet && $input_file ) {
		print "marginal probs inputted from file $input_file\n";
	}
	if( !$quiet && $wndchrm_args ) {
		print "wndchrm arguments: $wndchrm_args\n";
	}
	if( !$quiet && $granularity ) {
		print "input image will be sampled maximum $granularity times over defined end points.\n";
	}

	my $test_image_shell = $input_image;
	my $test_image_reg_exp = $input_image;

	$test_image_shell =~ s/([ \(\)])/\\$1/g; # for shell, spaces and parentheses need to be escaped
	$test_image_reg_exp =~ s/([\(\)\.])/\\$1/g; # for regular expressions, parens and periods need to be escaped

	my( $image_width, $image_height);
	my $retval = 0;
	my @output = `tiffinfo $test_image_shell`;
	foreach (@output) {
		if( /Image Width: (\d+) Image Length: (\d+)/ ) {
			print "Image Width: $1, height $2\n" if !$quiet;
			$image_width = $1;
			$image_height = $2;
			last;
		}
	}
	$retval = $? >> 8;
	print "tiff info return val = $retval\n" if !$quiet;
	if( !defined $image_width || !defined $image_height ) {
		die "Error reading image dimensions from input image $test_image_shell\n";
	}

	if( $input_file || $full_scan ) {
		if( $heatmap_image_path && $heatmap_image_path ne "" ) {
			if( $heatmap_image_path =~ /tif$|tiff$|png$/ ) {
				print "Output heatmap image will be saved to file $heatmap_image_path\n" if !$quiet;
			}
			else {
				warn "Filename $heatmap_image_path is invalid: Must end with a .tif, .tiff, or .png extension. Using default name \"image.tif\"\n";
				$heatmap_image_path = "image.tif";
			}
		}
		else {
			print "Output heatmap image will be saved to default file name ./image.tif\n" if !$quiet;
			$heatmap_image_path = "image.tif";
		}

		if( $input_file ) {
			@results_matrix = LoadInputFile( $input_file, $quiet );
		}
		else {
			@results_matrix = FullKernelScan( $path_to_wndchrm, $test_image_shell, 
				$test_image_reg_exp, $image_width, $image_height, $training_fit, $kernel_width, $kernel_height, $deltaX, $deltaY, $starting_percentage, $wndchrm_args, $quiet);
		}
		print "Loaded results matrix with $#results_matrix columns and ";
		print "$#{ $results_matrix[0] } rows.\n" if !$quiet;

		WriteOutputFile( \@results_matrix, $output_file ) if( $output_file );
		OutputASCIIpic( \@results_matrix );
		writeImage( \@results_matrix, $heatmap_image_path );
	}
	elsif( $start_end_line_scan || $start_displacement_line_scan ) {
		@results_matrix = LinearKernelScan( $start_end_line_scan, $path_to_wndchrm, $test_image_shell, $test_image_reg_exp, $image_width, $image_height, $training_fit, $kernel_width, $kernel_height, $x1, $y1, $x2, $y2, $deltaX, $deltaY, $granularity, $starting_percentage, $wndchrm_args, $quiet);
		print "Desired num samples was $granularity, actual num samples " .
				 ( 1 + $#results_matrix ) . " times.\n" if !$quiet;
		PrintLinearKernelScan( \@results_matrix, $granularity );
	}

	return 1;
}


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
# #FIXME: The ASCII output is hardcoded for a 5 class problem,
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
				print 'C';
			} elsif( $index_of_highest == 3 ) {
				print 'H';
			} elsif( $index_of_highest == 4 ) {
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
# #FIXME: This is also a 5-class application specific function
#################################################################################
sub LoadInputFile {
	my $input_file = shift;
	my $quiet = shift;
	my @results_matrix;

	open( IN, '<', $input_file ) or die "Couldn't open input file $input_file: $!\n";

	# ex: "col 7, row 4: (0.000	0.810	0.188	0.003	)"

	while( <IN> ) {
		if( /col (\d+), row (\d+): \((\d\.\d+)\s+(\d\.\d+)\s+(\d\.\d+)\s+(\d\.\d+)\s+(\d\.\d+)/ ) {
			print "loading col $1, row $2, marg probs( $3, $4, $5, $6, $7 )\n" if !$quiet;
			@{ $results_matrix[$1][$2] } = ( $3, $4, $5, $6, $7 );
		}
	}
	close IN;
	return @results_matrix;
}

#################################################################################
# WriteOutputFile( \@matrix, $output_file )
#################################################################################
sub WriteOutputFile {
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
#
#################################################################################
sub RunWNDCHARM_atROI {
	
	my ( $path_to_wndchrm, $test_image_shell, $test_image_reg_exp, $wndchrm_args, $x, $y, $kernel_width, $kernel_height, $training_fit, $quiet ) = @_;

	my $cmd = "$path_to_wndchrm classify $wndchrm_args -s1 -B$x,$y,$kernel_width,$kernel_height $training_fit $test_image_shell 2>&1";
	print "Running wndchrm command:\n $cmd \n" if !$quiet;
	my @output = `$cmd`;
	my $retval = $? >> 8;

	if( $retval != 1 ) {
		warn "WNDCHRM returned error: $retval\n";
		warn "Wndchrm output:\n";
		foreach (@output) { warn "$_\n"; }
		warn "\n";
		die;
	}
	my @return_ary = ();

	#print "Here was the output: $output\n\n";
	foreach (@output) {
		#print;
		if( /^$test_image_reg_exp\s+\S+\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/ ) {
			print "Caught marginal probabilities $1, $2, $3, $4, $5\n" if !$quiet;
			 @return_ary = ($1, $2, $3, $4, $5);
		}
	}
	die "Didn't catch any marginal probs\n" if( $#return_ary <= 0 );
	return @return_ary;
}
#################################################################################
# FullKernelScan()
#   required inputs: $path_to_wndchrm, $image_name, $training_set_fit_file, $deltaX, $deltaY
#   optional inputs: $starting_fraction
#   output:          @results_matrix
#################################################################################
sub FullKernelScan {

	my( $path_to_wndchrm, $test_image_shell, $test_image_reg_exp, $image_width, $image_height, $training_fit, $kernel_width, $kernel_height, $deltaX, $deltaY, $starting_percentage, $wndchrm_args, $quiet )= @_;

	if( !defined $wndchrm_args ) { $wndchrm_args = ""; }
	
	my @results_matrix;

	my $num_cols = int( ($image_width - $kernel_width) / $deltaX );
	my $num_rows = int( ($image_height - $kernel_height) / $deltaY );

	my $starting_col = int( $starting_percentage * $num_cols );
	my $remainder = ( $starting_percentage * $num_cols ) - $starting_col;
	my $starting_row = int( $remainder * $num_rows );

	print "Image will be scanned with $num_cols columns and $num_rows rows, starting at column $starting_col\n" if !$quiet;
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
			print "col $col, row $row, x: $x, y: $y, kernel width: $kernel_width, kernel height: $kernel_height\n" if !$quiet;
			@{ $results_matrix[$col][$row] } = RunWNDCHARM_atROI( $path_to_wndchrm, $test_image_shell, $test_image_reg_exp, $wndchrm_args, $x, $y, $kernel_width, $kernel_height, $training_fit, $quiet );
		}
	}
	return @results_matrix;
}

#################################################################################
#
#################################################################################
sub LinearKernelScan {
	my( $is_endpoints_scan, $path_to_wndchrm, $test_image_shell, $test_image_reg_exp, $image_width, $image_height, $training_fit, $kernel_width, $kernel_height, $x1, $y1, $x2, $y2, $deltaX, $deltaY, $num_samples, $starting_percentage, $wndchrm_args, $quiet )= @_;

	if( !defined $num_samples ) {
		if( !defined $deltaX and !defined $deltaY ) {
			die "To run a linear kernel scan, you must specify a \"number of samples\" \n value using the --g argument or a deltaX/deltaY step parameter using the --p argument.\n";
		}
	}

	if( !defined $wndchrm_args ) { $wndchrm_args = ""; }

	# Start and end coordinates are specified by the user as the center of
	# the kernel. These coordinates must be translated to the upper left corner
	# because that's how wndchrm specifies a sub image.
	# The x offset will be to subtract half the kernel width, and the y offset will
	# be to subtract half the kernel height.
	my $x_offset = 0.5 * $kernel_width;
	my $y_offset = 0.5 * $kernel_height;

	my( $rise, $run );
	if( $is_endpoints_scan ) {
		$rise = $y2 - $y1;
		$run = $x2 - $x1;
	}
	my( $x, $y, $x_topleft, $y_topleft, $x_botright, $y_botright );
	my $previousX = -1;
	my $previousY = -1;
	my @results_table;
	my @coordinate_pairs;



	while (1) {

		# use sprintf for proper rounding of pixel coordinates
		if( $is_endpoints_scan ) {
			$x = sprintf( "%.0f", ( $x1 + $run * $index / ($num_samples-1) ) );
			$y = sprintf( "%.0f", ( $y1 + $rise * $index / ($num_samples-1) ) );
		} else {
			$x = $x1 + ($index * $deltaX);
			$y = $y1 + ($index * $deltaY);
		}
		$x_topleft = $x - $x_offset;
		$y_topleft = $y - $y_offset;
		$x_botright = $x + $x_offset;
		$y_botright = $y + $y_offset;

		if( $x_topleft < 0 or $y_topleft < 0 ) {
			warn "Kernel center location of ($x,$y) with a kernel size of ($kernel_width,$kernel_height) results in a top left location of ($x_topleft,$y_topleft) which is past the edge of the image. Skipping...\n";
			next;
		}
		if( $x_botright > $image_width or $y_botright > $image_height) {
			warn "Kernel center location of ($x,$y) with a kernel size of ($kernel_width,$kernel_height) results in a bottom right corner location of ($x_botright,$y_botright) which is past the edge of the image. Skipping...\n";
			next;
		}

		next if( $x == $previousX and $y == $previousY );
		push @coordinate_pairs, { x => $x, y => $y };

		$previousX = $x;
		$previousY = $y;
		last if ;
	}

	my $starting_index = $num_samples * $starting_percentage;

	for( my $i = $starting_index; $i <= $#coordinate_pairs; $i++ ){
		@{ $results_table[$index]{"marg_probs"} } = RunWNDCHARM_atROI( $path_to_wndchrm, $test_image_shell, $test_image_reg_exp, $wndchrm_args, $coordinate_pairs[$i]{x}, $coordinate_pairs[$i]{y}, $kernel_width, $kernel_height, $training_fit, $quiet );
		$results_table[$index]{'x'} = $coordinate_pairs[$i]{x} + $x_offset;
		$results_table[$index]{'y'} = $coordinate_pairs[$i]{y} + $y_offset;
	}
	return @results_table;
}
#################################################################################
#
#################################################################################
sub PrintLinearKernelScan {
	my $results_matrix_ref = shift;

	print "Results:\n";
	for( my $i = 0; $i <= $#{ $results_matrix_ref }; $i++ ) {
		print "Samp ". ($i+1) . "\tx:$$results_matrix_ref[$i]{'x'}\ty:$$results_matrix_ref[$i]{'y'}\t";
		foreach ( @{ $$results_matrix_ref[$i]{'marg_probs'} } ) { print $_ . "\t"; };
		print "\n";
	}
}

#################################################################################
#
#################################################################################
sub ShowHelp {
	print "WNDCHRM heatmap generator, ver 1.30\n";
	print "\n";
	print "Required arguments:\n";
	print "  --i /path/name - Input image: the image which will be scanned.\n";
	print "  --w /path/name - WNDCHRM path: path to the wndchrm executable.\n";
	print "  --t /path/name - Training Set: path to the .fit file containing the classifier.\n";
	print "                     must be generated beforehand usind WNDCHRM\n";
	print "  --k <#x#>        Kernel_size: indicate size of scanning window\n";
	print "\n";
	print "Operation 1: Perform a full window scan on entire image.\n";
	print "  --p <#x#>      - Specify horizontal and vertical displacement of scanning window.\n";
	print "  ex: heatmap.pl --i input_image.tiff --w /path/to/wndchrm --t /path/to/training_set.fit -k 280x280 -p 20x20\n";
	print "\n";
	print "Operation 2: Perform a linear kernel scan over input image using a line defined by start\n";
	print "             coordinates, kernel displacement, and desired number of samples.\n";
	print "             Output is a table of marg. probs.\n";
	print "  --b <#,#>      - Beginning of line pixel coordinates x1=# and y1=#\n";
	print "  --p <#x#>      - Specify horizontal and vertical displacement of scanning window.\n";
	print "  --g #          - Number of samples (number of window displacements)\n"; 
	print "  ex: heatmap.pl --i input_image.tiff --w /path/to/wndchrm --t /path/to/training_set.fit --k 280x280 --b 123,456 --p 20x0 -g 30\n";
	print "\n";
	print "Operation 3: Perform a linear kernel scan over input image using a line defined by start\n";
	print "             and end coordinates, and the desired number of samples. Samples are equidistantly\n";
	print "             spaced along line. Output is a table of marg. probs.\n";
	print "  --b <#,#>      - Beginning of line pixel coordinates x1=# and y1=#\n";
	print "  --e <#,#>      - End of line pixel coortinates x2=# and y2=#\n";
	print "  --g #          - Number of samples (number of window displacements)\n"; 
	print "  ex: heatmap.pl --i input_image.tiff --w /path/to/wndchrm --t /path/to/training_set.fit --k 280x280 --p 20x20 --b 123,456 --e 789,1011 --g 25\n";
	print "\n";
	print "Additional optional arguments:\n";
	print "  --h /path/name - Specify a filename and path to the generate heatmap image.\n";
	print "                     otherwise, image is created with deafult name \"image.tif\"\n";
	print "  --d /path/name - Dump marginal probabilities to file to save time when re-running\n";
	print "                     this script later.\n";
	print "  --l /path/name - Load a file containing dumped marginal probabilities created\n";
	print "                     by previously running this script using the -d option.\n";
	print "                     Note: does not require --i, --w, --t, and --k parameters\n";
	print "                     ex: heatmap.pl -l some_dumpfile.txt -h new_heatmap.png\n";
	print "  --s <0.#>      - Starting point: Used to employ multiple processors to train signatures.\n";
	print "                     Specify a decimal from 0.0 to 1.0 indicating where in the image this\n";
	print "                     instance should start calculating signatures, e.g., start halfway through\n";
	print "                     (--s 0.5) or start three-quarters of the way through (--s 0.75) etc.\n";
	print "  --a \"string\"   - Specify wndchrm command line arguments, e.g., -a \"-l -f0.03\"\n";
	print "  --q            - Quiet: print only essential information to console\n";
}
