#!/usr/bin/perl -w
# Written by Chris Coletta christopher.coletta@nih.gov
# 23 November 2009
#
# This script is meant to function as a smoke test for a successfully
# compiled wndchrm.
#
# Meant to be fired off in the directory that wndchrm was compiled.
# Arguments
# $ARGV[0] - optional - a string containing a date or something similarly distinctive
#       that can be used to concatenate onto various output files generated here.
# Requirements
# There are supplementary files that are required to run through wndchrm and test the output
# terminal_bulb_feature_set_cCONTROL - precalculated feature file used by this script to
#                run a wndchrm test

# More information about the Test::More usage and syntax is available in the
# Test::More documentation. ( Try `perldoc Test::More` on your *nix command line )
use Test::More; 
note( "BEGIN WNDCHRM SMOKE TEST" );

use Getopt::Long;
my $date_suffix = ""; 
my $base_test_dir = "../..";
GetOptions( "suffix=s"  => \$date_suffix, # =s implies the value is a string
            "basedir=s" => \$base_test_dir);

note( "Date suffix: $date_suffix" );
note( "base test dir: $base_test_dir" );

# Did wndchrm compile?
ok( -e "./wndchrm", "Check for the executable" )
  or BAIL_OUT( "Fatal error: no executable to run test!!!" );

# Parse the file to see if the numbers are acceptable
#subtest  "Testing the wndchrm test functionality on precomputed C. Elegans feature file." => sub
#  {
    note( "Begin testing HTML test file." );  
    # Parse the file to see if the numbers are acceptable
    
    # Here the stdout of running wndchrm is piped to a file for record keeping
    # Note the directory structure, which should have been created by the 
    # checkout script.
    #   
    # TODO: Make this test directory structure independent.
    #   
    my $output_file = "$base_test_dir/output_products/wndchrm_smoke_test_terminal_bulb$date_suffix.html";
    my $cmd_str = "./wndchrm test -i40 -j14 -n10 ~/read_only_files/terminalbulb_feature_set_CONTROL $output_file > $base_test_dir/output_products/terminal_bulb_test_STDOUT"; 
    note( $cmd_str );  
    system( $cmd_str );
    cmp_ok( $?, "!=", -1, "Check to see that Perl was able to fire off wndchrm test command." );
    ok( -e $output_file, "Check to see if Terminal Bulb test HTML results file was created." );

    SKIP:
      {
        eval( require HTML::TreeBuilder );
          skip "No HTML::TreeBuilder module installed. Please install it from CPAN.ORG and rerun test." if $@;

        if( ! -e $output_file )
        {
          BAIL_OUT( "No file to parse!!!" );
        }

        my $tree = HTML::TreeBuilder->new_from_file( $output_file );
        isa_ok( $tree, HTML::TreeBuilder, "Check if Perl could parse the HTML file." );

        #$tree->dump; 
        my $overall_test_results_element = $tree->look_down("id", "overall_test_results" );
        if( !defined $overall_test_results_element)
          {
            fail( "Couldn't find the overall test results element" );
          }
        else
          {
            pass( "Found test results element" );
            my $overall_test_results_text = $overall_test_results_element->as_text();
            note( $overall_test_results_text );
            if( ok( $overall_test_results_text =~ /^\s*(\d*\.\d*)/ ,
                    "Find an overall percentage of successful classification.") )
            {
              ok( $1 > .3 && $1 < .5 ,
                  "Check to see if overall classification success is within .25-.55" );
            }
          }
        $tree->delete();
      } # end SKIP
# done_testing();  
#  }; # end subtest

done_testing();


__END__

