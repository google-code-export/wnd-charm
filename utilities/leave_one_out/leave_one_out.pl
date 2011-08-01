#!/usr/bin/perl

use warnings;
use strict;
#require HTML::Tree;

sub uniq {
    return keys %{{ map { $_ => 1 } @_ }};
}

my $orig = shift;
print $orig . "\n";

my $wndcharm_path = "~/src/wnd-charm.googlecode.com/wnd-charm/wndchrm/tags/wndchrm-1.31/wndchrm";
my $cmd;

die "must provide master wndcharm input file of files\n" if (!defined $orig);
my $orig_base = $orig;
$orig_base =~ s/\.txt//;

my $container_dir = "${orig_base}_unbalanced_files";
mkdir $container_dir;

my %groups;
my %class_list;
my $basename;

open IN, $orig or die;
foreach my $line (<IN>) {
	if ( $line =~ /(.*)_CR.*\t(\S+)$/ ) {
		push @{ $groups{$1} }, $line;
		$class_list{$1} = $2;
	}
}
close IN;

open OUT, ">", "${orig_base}_aggregated_results_unbalanced.html" or die;

my $count = 0;
foreach my $class ( sort &uniq( values %class_list ) ) {
	print OUT "<p><h1>Results from class $class</h1></p>\n";

	foreach my $group_name (sort keys %groups) {
		next if( $class_list{$group_name} ne $class ); 

		#make a dir inside container_dir for each group
		mkdir "$container_dir/$group_name";

		my $training_file = "$container_dir/$group_name/${orig_base}_${group_name}_left_out.txt";
		my $test_file = "$container_dir/$group_name/${group_name}_only.txt";
		my $test_html = "$container_dir/$group_name/${group_name}_only.html";

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

		$cmd = "$wndcharm_path classify -l -r#1 $training_file $test_file $test_html";
		system( $cmd );
		
		print OUT "<p><h3>" . ++$count . ". from $test_html</h3></p>\n";
		#my $tree =  HTML::TreeBuilder->new_from_file( $test_html );
		#die if !$tree;

		#my $element = $tree->look_down( "id", "master_confusion_matrix" );
		#die if !$element;

		open IN, $test_html or die;

		my $report_beginning_section = "";
		while (<IN>) {
			if( /NAME=\"split0\"/ ) {
				last;
			}
			
			$report_beginning_section .= $_;
		}

		close IN;

		#print OUT $element->as_HTML;
		if( $report_beginning_section =~ /(<table id=\"master_confusion_matrix\".*?table>)/s ) {
			print OUT $1;
		} else {
			die "can't find master confusion matrix";
		}


		#$element = $tree->look_down( "id", "average_class_probability_matrix" );
		#die if !$element;
		#print OUT $element->as_HTML;

		if( $report_beginning_section =~ /(<table id=\"average_class_probability_matrix\".*?table>)/s ) {
			print OUT $1;
		} else {
			die "can't find average_class_probability matrix";
		}
		
		#$tree->delete();
	}
}

close OUT;

# collect all results 
