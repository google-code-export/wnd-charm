#!/usr/bin/perl
#
# Phase 3.pl
# Custom script written for John Delaney's pairwise wndchrm distance gene experiment
#
# Pseudocode:
# 1. scoop up all the htmlfiles
# 2. Parse the filename to ascertain which pairwise experiment it is
# 3. Parse the file itself for the information
# 4. Perform the calculation, and store in an orderly way
# 5. After all files have been parsed, generate master htmlfile from data

use strict;
use warnings;
use lib qw( /home/colettace/phase3_perl_modules );
use HTML::Tree;


my @htmlfile_list = split( "\n", `find . -name "*.html"` );
my %class_info; # Two dimensional hash stores the names and pairwise distances 
my %uncertainties; # Another two dimentional hash where each matrix position corresponds with
                   # the %class_info position. The coefficients are uncertatinty for that gene pair.
if( $#htmlfile_list <= 0 ) {
	die "Dude, no friggin dend files in this dir, you idiot!\n";
}

#my $DEBUG =0;
#if( $DEBUG ) {
#	foreach (@htmlfile_list) {
#		print $_ ."\n";
#	}
#}

my $DEBUG0 = 0;

for( my $i = 0; $i <= $#htmlfile_list; $i++) {
	# Get rid of path
	my $html_file = $htmlfile_list[$i];
	$html_file =~ s/.+\///g;
	if( $html_file !~ /_(.+)[-\+]D_VS_(.+)[-\+]D\.html/ ) {
		die "Error, process aborted: $html_file doesn't follow naming convention of \"...<gene1>-D_VS_<gene2>-D\"\n";
	}
	print "html file $html_file has genes $1 and $2.\n"; # if( $DEBUG0 );
	my $gene1 = $1;
	my $gene2 = $2;
	my $tree = HTML::TreeBuilder->new_from_file( $htmlfile_list[$i] );
	# Look down into the HTML tree for a table with the id =
	# "average_class_probability_matrix"
	#my $class_pm_table_element = $tree->look_down("id", "average_class_probability_matrix" );
	#$tree->dump;
	my $class_pm_table_element = $tree->look_down("id", "average_class_probability_matrix" );
	if( !defined $class_pm_table_element ) {
		die "Didn't find the average class probability table for file $html_file!!\n";
	}

	my $using_error_bars = 0;
	my @dm = ();
	my @dm_errors = ();
	my @vectorON = ();
	my @vectorOFF = ();
	my @cv = ();
	my $correctedvector_MAG = 0;
	my @rows = $class_pm_table_element->look_down("_tag", "tr");
	# Skip the first row, i.e., start at 1 not 0,
	# because that just where the column headings are
	for( my $i = 1; $i <= $#rows; $i++) {
		my @cols = ();
		@cols = $rows[$i]->look_down("_tag","td");
		foreach (@cols) {
			my $val = $_->as_text;
			if( $val =~ /(\d?\.\d+) +\/- (\d?\.\d+)/)
			{
				$using_error_bars = 1;
				my $mean = $1;
				my $error_bar = $2;
				push @{ $dm[$i-1] }, $mean;
				push @{ $dm_errors[$i-1] }, $error_bar;
			}
			else
			{
				push @{ $dm[$i-1] }, $val;
			}
		}
	}
	$tree->delete();
	
	# Print out the matrix we just absorbed from the html file
	print "\nMarginal probability data:\n";
	foreach my $row ( @dm ) {
		print "\t";
		foreach my $avg_marg_prob ( @{ $row } ) {
			print $avg_marg_prob . " ";
		}
		print "\n";
	}

	if( $using_error_bars ) {
		print "\nMatrix of uncertainties:\n";
		foreach my $row ( @dm_errors ) {
			print "\t";
			foreach my $avg_marg_prob ( @{ $row } ) {
				print $avg_marg_prob . " ";
			}
			print "\n";
		}

	}
	#print " 3.0 (" . $dm[3][0]; print ")- 1.0 (" . $dm[1][0]; print "), 3.1 (" . $dm[3][1]; print ") - 1.1 (" . $dm[1][1]; print "), 3.2 (" . $dm[3][2]; print ")- 1.2 (" . $dm[1][2]; print "), 3.3 (" . $dm[3][3]; print ")- 1.3 (" . $dm[1][3] . ") = ";
	@vectorON = ( ($dm[3][0]-$dm[1][0]), ($dm[3][1]-$dm[1][1]), ($dm[3][2]-$dm[1][2]), ($dm[3][3]-$dm[1][3]) );
	print "\tGENE ON vector (row 4 - row 2):"; foreach (@vectorON) { print $_ . ", "; }; print "\n";
	@vectorOFF =( ($dm[2][0]-$dm[0][0]), ($dm[2][1]-$dm[0][1]), ($dm[2][2]-$dm[0][2]), ($dm[2][3]-$dm[0][3]) );
	print "\tGENE OFF vector (row 3 - row 1): "; foreach (@vectorOFF) { print $_ . ", "; }; print "\n";
	@cv = ( ($vectorON[0]-$vectorOFF[0]), ($vectorON[1]-$vectorOFF[1]), ($vectorON[2]-$vectorOFF[2]), ($vectorON[3]-$vectorOFF[3]) );
	print "\tresult vector (ON-OFF): "; foreach (@cv) { print $_ . ", "; } print "\n";
	$correctedvector_MAG = sqrt( $cv[0]**2 + $cv[1]**2 + $cv[2]**2 + $cv[3]**2 );
	print "\tmagnitude of result vector: " . $correctedvector_MAG . "\n";
	$class_info{$gene1}{$gene2} = $correctedvector_MAG;
	$class_info{$gene2}{$gene1} = $correctedvector_MAG;

	my $propagated_uncertainty = 0;

	foreach( @dm_errors ) {
		$propagated_uncertainty += ($_)**2;
	}
	$propagated_uncertainty = sqrt( $propagated_uncertainty );
	$uncertainties{$gene1}{$gene2} = $propagated_uncertainty;
	$uncertainties{$gene2}{$gene1} = $propagated_uncertainty;

}
open OUTPUT, ">master_dendfile_CLASS_PROBABILITIES.txt" or die "Error, process aborted: Can't open output file: $!\n";

my @master_gene_list = sort keys %class_info;
my $thecount = $#master_gene_list + 1;
print OUTPUT "$thecount\n";

my $row;
my $col;

foreach $row ( @master_gene_list ) {
	printf( OUTPUT "%s                 ", $row);
	foreach $col ( @master_gene_list ) {
		if( defined $class_info{$row}{$col} ) {
			printf OUTPUT "%0.4f       ", $class_info{$row}{$col};
		} else {
			print OUTPUT "0.0000       ";
		}
	}
	print OUTPUT "\n";
}

close OUTPUT;

open OUTPUT, ">master_dendfile_CLASS_PROBABILITIES_WITH_UNCERTAINTIES.txt" or die "Error, process aborted: Can't open output file: $!\n";

print OUTPUT "$thecount\n";

foreach $row ( @master_gene_list ) {
	printf( OUTPUT "%s                 ", $row);
	foreach $col ( @master_gene_list ) {
		if( defined $class_info{$row}{$col} ) {
			printf OUTPUT "%0.4f +/- %0.4f   ", $class_info{$row}{$col}, $uncertainties{$row}{$col};
		} else {
			print OUTPUT "0.0000             ";
		}
	}
	print OUTPUT "\n";
}

close OUTPUT;

