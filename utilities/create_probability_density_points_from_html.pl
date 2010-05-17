#!/usr/bin/perl
use strict;
use warnings;
require HTML::Tree;
require Statistics::Descriptive;

use Getopt::Long;
my $num_bins = 20;
#my $bin_width = undef;
my $normalize = undef;
my $num_classes = undef;
my $flags = undef;
GetOptions( "bins=i"  => \$num_bins,
           # "window_size=f" => \$bin_width,
            "normalize=s" => \$normalize,
            "num_classes=i" => \$num_classes,
            "flags=i" => \$flags );

print "Number of bins: $num_bins\n" if( defined $num_bins );
#print "Window size : $bin_width\n" if( defined $bin_width );
print "Age scores will be normalized\n" if( defined $normalize );

# Takes a wndchrm output html file as input.
# HTML file should have been procured as part of a wndchrm test operation
# where the operation derives interpolated values

# This code parses html output file, and counts how many images in the test set were
# sorted into the given bins.
my $output_file = shift;

print "Loading $output_file\n";
my $tree = HTML::TreeBuilder->new_from_file( $output_file );
print "Done loading $output_file\n";

#$tree->dump; 
my @table_elements = $tree->look_down("_tag", "TABLE", 
     sub
     {
       if( defined $_[0]->attr("ID") ) {
         if( $_[0]->attr("ID") =~ /IndividualImages_split/ ) {
           # print "Got one!\n";
           return 1;
         }
       }
       # print "DON' GOT ONE!!\n";
       return 0;
     }
   );

die "Couldn't find the test results table element in file $output_file\n" if( !@table_elements );
# print "Number of splits found: $#table_elements\n";

# step one: figure out the number of classes
if( !defined $num_classes ) {
  my $class_structure_elem = $tree->look_down("_tag", "table");
  die "Couldn't derive number of classes from html file $output_file.\n" if( !$class_structure_elem );

#  print $class_structure_elem->as_text . "\n";

  my $first_row = $class_structure_elem->look_down("_tag", "tr"); #grab the first one it sees
  die "Couldn't derive number of classes from html file $output_file.\n" if( !$first_row );

# print $first_row->as_text . "\n";

  my @class_rows = $first_row->look_down("_tag", "td");
  my $num_classes = $#class_rows - 1;
}
print "Number of classes used: $num_classes\n";

my @rows;
my @row;
my %results_hash;
my $val;
my $img_link_element;
my $fullpath;
my $filename;
my ($actual_class, $predicted_class, $interpolated_value);
my $DEBUG1 = 0;

my $normalized_distances;
my $split_number;

my $min = -1;
my $max = -1;
my $image_column;

foreach my $split_table_element (@table_elements)
{
  @rows = $split_table_element->look_down("_tag", "tr");

  # print "Parsing $output_file, " . $split_table_element->attr("ID") ."\n";
  if( $split_table_element->attr("ID") =~ /IndividualImages_split(\d+)/ ) {
    $split_number = $1;
  }
  else {
    $split_number = -1;
  }

  # Parse the first "Caption" row
  @row = $rows[0]->look_down("_tag", "td");
  my $caption_text = $row[$#row]->as_text;
  if( $caption_text =~ /Most similar image/ ) {
    $image_column = $#row - 1;
  } else {
    $image_column= $#row;
  }
  if( $flags =~ /1/ ) {
# weird wndchrm report discrepancy, use second row when column headers don't match up.
    @row = $rows[1]->look_down("_tag", "td");
    $image_column= $#row;
  }

  print "Image column is $image_column\n\n" if( $DEBUG1 );


# The first row is the heading row, so skip it by starting at 1 instead of 0
  for( my $i = 1; $i <= $#rows; $i++) {
    $val = 0;
    @row = ();
    $img_link_element = undef;
    @row = $rows[$i]->look_down("_tag","td");
    if( $DEBUG1 ) {
      foreach (@row) {
        print "  " . $_->as_text;
      }
      print "\n";
    }
    $img_link_element = $row[$image_column]->look_down( "_tag", "A" );
    $fullpath = $img_link_element->attr("HREF");
    if( $fullpath =~ /\S*\/(\S+)/ ) {
      $filename = $1;
#      print "\tFound file $filename\n";

      $normalized_distances = "";
      for( my $j = 3; $j <= ( 3 + $num_classes ); $j++ ) {
        $normalized_distances .= $row[$j]->as_text . "  ";
      }
      $actual_class = $row[ $image_column - 4 ]->as_text;
      $predicted_class = $row[ $image_column - 3 ]->as_text;
      $interpolated_value = $row[ $image_column - 1 ]->as_text;
      if( $min == -1 ) {
        $min = $interpolated_value;
      }
      $min = $interpolated_value if( $interpolated_value < $min );
      if( $max == -1 ) {
        $max = $interpolated_value;
      }
      $max = $interpolated_value if( $interpolated_value > $max );
      print "\t\tactual: $actual_class, predicted: $predicted_class, interp val: $interpolated_value\n" if( $DEBUG1 );
      push @{ $results_hash{ $actual_class }->{ $filename } }, { "split_num" => $split_number, "val" => $interpolated_value, "class" => $predicted_class, "dists" => $normalized_distances };
    }
  }
}
$tree->delete();


my $range = $max - $min;
my @interp_vals;
my $stat = Statistics::Descriptive::Sparse->new();
my $norm_stat = Statistics::Descriptive::Sparse->new();
my $class_stat = Statistics::Descriptive::Full->new();
my $report;
my $graph_this;
my $distribution_hash;

#print "RESULTS:\n";
foreach my $class ( sort keys %results_hash ) {
	#print "\tClass \"$class\"\n";
  $class_stat->clear;
  $distribution_hash = undef;
  foreach my $file (keys %{ $results_hash{ $class } }) {
    @interp_vals = ();
    # print "\t\tFile \"$file\"\n";
    $stat->clear;
    $norm_stat->clear;
    foreach my $hash_ref ( @{ $results_hash{ $class }->{ $file } } ) {
      $interpolated_value = $hash_ref->{ "val" };
      $stat->add_data( $interpolated_value );
      $norm_stat->add_data( ( $interpolated_value - $min ) / $range );
      $predicted_class = $hash_ref->{ "class" };
      $split_number = $hash_ref->{ "split_num" };
      $normalized_distances = $hash_ref->{ "dists" };
#      printf "\t\t\tsplit %2.d - predicted: $predicted_class, actual: $class. Normalized dists: ( $normalized_distances ) Interp val: $interpolated_value\n", $split_number;
    }
#    printf "\t\t\t---> Tested %d times, mean %.3f, std dev %.3f. Normalized to [0,1]: mean: %.4f, std_dev: %.4f\n\n",
    $stat->count, $stat->mean, $stat->standard_deviation, $norm_stat->mean, $norm_stat->standard_deviation; 
    if( defined $normalize ) {
      $class_stat->add_data( $norm_stat->mean );
    } else {
      $class_stat->add_data( $stat->mean );
    }
  }
  $class_stat->sort_data;
  $report .= sprintf "Class $class: count= %3d, min=%.4f, max=%.4f, mean=%.4f, std dev=%.4f\n", 
               $class_stat->count, $class_stat->min, $class_stat->max, $class_stat->mean, $class_stat->standard_deviation;
#   if( $bin_width ) {
#     print "using bin distribution array\n";
#     $distribution_hash = $class_stat->frequency_distribution_ref(  );
#   }
#  elsif( $num_bins ) {
#    print "using $num_bins number of bins\n";
    $distribution_hash = $class_stat->frequency_distribution_ref($num_bins);
#  }

  $graph_this .= "Class $class\n";
  foreach( sort {$a <=> $b} keys %$distribution_hash ) {
    $graph_this .= "$_\t$distribution_hash->{$_}\n";
  }
}
print "\nGlobal min val: $min, global max val: $max\n";
print "\n\n***********REPORT********\n\n" . $report . "\n";
print "\n\n**********PLOT THESE********\n\n" . $graph_this . "\n\n\n";

