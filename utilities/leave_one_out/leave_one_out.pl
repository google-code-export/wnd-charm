#!/usr/bin/perl

use warnings;
use strict;

sub uniq {
    return keys %{{ map { $_ => 1 } @_ }};
}

my $usage = "\nUSAGE (three mandatory arguments):\nleave_one_out.pl <file of files> <full path to wndcharm> \"<[test|classify] wndcharm parameters>\"\n";
my $orig = shift; #monkey_brain_CA1.txt
if( !defined $orig or $orig eq "" ){
	die "error with file of files\n$usage";
}
print "Reading in file $orig\n";

my $wndcharm_path = shift; #"/Users/chris/src/wnd-charm.googlecode.com/wnd-charm/wndchrm/trunk/wndchrm";
if( !defined $wndcharm_path or $wndcharm_path eq "" ) {
	die "error with wndcharm path\n$usage";
}
print "Using path to wndcharm: $wndcharm_path\n";

my $wndcharm_params = shift; # "test -l -r1 -i10 -n101 -S2201";
if( !defined $wndcharm_params or $wndcharm_params eq "" ) {
	die "error with wndcharm parameters\n$usage";
}
print "Using wndcharm parameters: $wndcharm_params\n";

my $cmd;

die "must provide master wndcharm input file of files\n" if (!defined $orig);
my $orig_base = $orig;
$orig_base =~ s/\.txt//;

my $container_dir = "${orig_base}_files";
mkdir $container_dir;

my %groups;
my %class_list;
my $basename;

open IN, $orig or die "Couldn't open $orig... dying.\n";
foreach my $line (<IN>) {
	if ( $line =~ /(.*)_CR.*\t(\S+)$/ ) {
		push @{ $groups{$1} }, $line;
		$class_list{$1} = $2;
	}
}
close IN;

my $output_filename = "${orig_base}_aggregated_results.html";

my $out_buffer;
open OUT, ">", \$out_buffer or die;

my $count = 0;
my $class_count;
my @class_confusion_matrix;
my @class_marg_probs;
my $table;
my $row;

my %confusion_matrix;
my %marg_prob_matrix;
my %num_individuals;

my $class;


foreach $class ( sort &uniq( values %class_list ) ) {
	print $class . "\n";
	$class_count = 0;
	@class_marg_probs = ();
	@class_confusion_matrix = ();
	print OUT "<hr><p><h2>Individual results from class $class</h2></p>\n";
	foreach my $group_name (sort keys %groups) {
		next if( $class_list{$group_name} ne $class );
		print "\t" . $group_name . "\n";

		++$class_count;
		$table = "";
		$row = "";

		#make a dir inside container_dir for each group
		mkdir "$container_dir/$group_name";

		my $training_file = "$container_dir/$group_name/${orig_base}_${group_name}_left_out.txt";
		my $test_file = "$container_dir/$group_name/${group_name}_only.txt";
		my $test_html = "$container_dir/$group_name/${group_name}_only.html";

		$cmd = "$wndcharm_path $wndcharm_params $training_file $test_file $test_html";
		my $report_beginning_section;

		print "\t\tlooking for $test_html...";
		if( -e $test_html ) {
			# Open the html and see if it was created with the same parameters specified here
			print " found. ";
			open IN, $test_html or die;

			$report_beginning_section = "";
			while (<IN>) {
				if( /NAME=\"split0\"/ ) {
					last;
				}
				$report_beginning_section .= $_;
			}
			# grab everything inside the command line <pre>....</pre>
			# skip leading whitespace
			if( $report_beginning_section =~ /<pre>\s*(.*?)<\/pre>/s ) {
				my $already_ran_command = $1;
				if( $already_ran_command ne $cmd ) {
					my $error_msg =  "***FATAL ERROR:\n";
					$error_msg .= "wndcharm has already been run for this data set but with different parameters:\n";
					$error_msg .= "You wanted this: \n";
					$error_msg .= "\t$cmd\n";
					$error_msg .= "You have this: \n";
					$error_msg .= "\t$already_ran_command\n";
					$error_msg .= "Please move the old dataset out of the way\n dying...\n";
					die	$error_msg;
				}
				else
				{
					print " Parameters match. \n"
				}
			}
		}
		else
		{
			# have to run wndcharm from scratch
			print " not found.\n";

			open LEFT_OUT, ">", $test_file or die;
			open ALL_ELSE, ">", $training_file or die;
			foreach my $group (keys %groups) {
				if( $group eq $group_name ) {
					foreach my $img_name ( @{ $groups{ $group } } ) {
						print LEFT_OUT $img_name;
					}
				}
				else {
					foreach my $img_name ( @{ $groups{ $group } } ) {
						print ALL_ELSE $img_name;
					}
				}
			}
			close LEFT_OUT;
			close ALL_ELSE;

			print "\n\n\n========================================================================\n";
			print $cmd . "\n\n";
			system( $cmd );
			open IN, $test_html or die;

			$report_beginning_section = "";
			while (<IN>) {
				if( /NAME=\"split0\"/ ) {
					last;
				}
				$report_beginning_section .= $_;
			}
		}
		
		print OUT "<p><h3>" . ++$count . ". from $test_html</h3></p>\n";
		print OUT "<p>Command:<br><pre>$cmd<\/pre><\/p>\n";

		close IN;
		print "\t\tparsing $test_html...\n";


		if( $report_beginning_section =~ /(<table id=\"master_confusion_matrix\".*?table>)/s ) {
			$table = $1;
			print OUT $table;
			# find the row in the matrix that contains the confusion numbers for this class only
			my $sum = 0; # used to avoid saving the sum total column in the confusion matrix
			if( $table =~ /<tr><th>$class<\/th>(.*?)<\/tr>/s ) {
				$row = $1;
				my $index = 0;
				print "\t\t\tconfusion matrix:\n\t\t\t";
				while( $row =~ /<td.*?>(\d+)<\/td>/p ) {
					last if( $1 == $sum ); # we reached the sum total column, just bail here
					print "$1 ";
					$sum += $1;
					$class_confusion_matrix[$index++] += $1;
					$row = ${^POSTMATCH};
				}
				print "\n";
			} else {
				die "didn't find class $class row in confusion matrix:\n\n$table\n";
			}
		} else {
			die "can't find master confusion matrix";
		}

		if( $report_beginning_section =~ /(<table id=\"average_class_probability_matrix\".*?table>)/s ) {
			$table = $1;
			print OUT $table;
			# find the row in the matrix that contains the avg class probs for this class only
			if( $table =~ /<tr><th>$class<\/th>(.*?)<\/tr>/s ) {
				$row = $1;
				my $index = 0;
				while( $row =~ /<td.*?>(\d\.\d+)<\/td>/p ) {
					$class_marg_probs[$index++] += $1;
					$row = ${^POSTMATCH};
				}
			} else {
				die "didn't find class $class row in the avg marg prob matrix:\n\n$table\n";
			}
		} else {
			die "can't find average_class_probability matrix";
		}
		
	} #end foreach individual

	$num_individuals{$class} = $class_count;

#	print OUT "<table border=\"1\" align=\"center\"><caption>Sum of all confusion matrices over population in class $class<\/caption>\n<tr>";
#	for(@class_confusion_matrix) {
#		print OUT "<td>" . $_ . "<\/td>";
#	}
#	print OUT "<\/tr><\/table>\n";
#
	@{ $confusion_matrix{$class} } = @class_confusion_matrix;
#
#	print OUT "<table border=\"1\" align=\"center\"><caption>Average of average marginal probabilities over population in class $class<\/caption>\n<tr>";
#	for( my $i = 0; $i <= $#class_marg_probs; ++$i ) {
#		$class_marg_probs[$i] = sprintf( "%.3f", $class_marg_probs[$i] / $class_count );
#		print OUT "<td>" . $class_marg_probs[$i] . "<\/td>";
#	}
#	print OUT "<\/tr><\/table>\n";

	@{ $marg_prob_matrix{$class} } = @class_marg_probs;
} #end foreach class

close OUT; # all the text printed to out is saved in $out_buffer

################################################################
# NOW PRINT REPORT

my $grand_total = 0;
my $grand_total_correct = 0;
my $class_total;
my $class_total_correct;

print "\n\n\n========================================================================\n";
print "Generating report $output_filename\n";
open OUTPUT_HTML, ">", $output_filename or die "Can't open output file $output_filename\n";
print OUTPUT_HTML "<html><body><h1>Leave-one-out results from dataset $orig<\/h1>\n";


# AGGREGATE CONFUSION MATRIX
print OUTPUT_HTML "<p><table border=\"1\" align=\"center\"><caption>Aggregate sum of all confusion matrices<\/caption>\n<tr><td><\/td>";

#column headers
#### BIG WARNING!!!! note the use of sort here ... will screw up order of column header captions
#### if the class names aren't sortable!!!!!!!!!!!!!
foreach $class ( sort keys %num_individuals ) {
	print OUTPUT_HTML "<th>$class<\/th>";
}
print OUTPUT_HTML "<th><\/th><th>class total<\/th><th>class accuracy<\/th><\/tr>\n";

$row = 0;
my $col = 0;
my $accuracy =0;
my $std_error_of_mean =0;
my $confidence_interval =0;

foreach $class ( sort keys %num_individuals ) {
	$class_total = 0;
	$class_total_correct = 0;
	$col = 0;
	print OUTPUT_HTML "<tr><th>$class ($num_individuals{$class} individuals)<\/th>";
	for( my $i = 0; $i <= $#{ $confusion_matrix{ $class } }; $i++ ) {
		my $val = $confusion_matrix{ $class }->[$i];
		print "class $class row $i val $val\n";
		$grand_total += $val;
		$class_total += $val;
		if( $row == $col ) {
			print OUTPUT_HTML "<td bgcolor=#D5D5D5>" . $val . "<\/td>";
			$grand_total_correct += $val;
			$class_total_correct += $val;
		}
		else {
			print OUTPUT_HTML "<td>" . $val . "<\/td>";
		}
		++$col;
	}
	$accuracy = $class_total_correct/$class_total;
	$std_error_of_mean = sqrt( $accuracy * (1-$accuracy) / $class_total );
	$confidence_interval = 1.95996 * $std_error_of_mean;

	print OUTPUT_HTML "<td><\/td><td>$class_total<\/td><td>" . sprintf( "%0.4f", $accuracy ) . " +/- " . sprintf( "%0.4f", $confidence_interval ) . "<\/td><\/tr>\n";
	++$row;
}

$accuracy = $grand_total_correct/$grand_total;
$std_error_of_mean = sqrt( $accuracy * (1-$accuracy) / $grand_total );
$confidence_interval = 1.95996 * $std_error_of_mean;

print OUTPUT_HTML "<\/table><br><div align=\"center\">Total accuracy: " .  sprintf( "%0.4f", $accuracy) . " +/- " . sprintf( "%0.4f", $confidence_interval ) . "<\/div><\/p>\n";


# AGGREGATE CLASS MARGINAL PROBABILITY MATRIX
print OUTPUT_HTML "<table border=\"1\" align=\"center\"><caption>Average of average class probabilities<\/caption>\n<tr><td><\/td>";

#column headers
#### BIG WARNING!!!! note the use of sort here ... will screw up order of column header captions
#### if the class names aren't sortable!!!!!!!!!!!!!
foreach $class ( sort keys %num_individuals ) {
	print OUTPUT_HTML "<th>$class<\/th>";
}
print OUTPUT_HTML "<\/tr>\n";

foreach $class ( sort keys %num_individuals ) {
	print OUTPUT_HTML "<tr><th>$class ($num_individuals{$class} individuals)<\/th>";
	foreach ( @{ $marg_prob_matrix{ $class } } ) {
		print OUTPUT_HTML "<td>" . $_ . "<\/td>";
	}
	print OUTPUT_HTML "<\/tr>\n";
}
print OUTPUT_HTML "<\/table><br><hr>\n";

# PRINT INDIVIDUAL OUTCOMES
print OUTPUT_HTML $out_buffer;

print OUTPUT_HTML "<\/body><\/html>\n";
close OUTPUT_HTML;



