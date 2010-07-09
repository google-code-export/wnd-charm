#!/usr/bin/perl
# Used to combine two or more WNDCHRM signature files into a single signature file
# thus combining the signal obtained from the various image channels.

use strict;
use warnings;

# Allows easy parsing of command line arguments
use Getopt::Long;

# Easiest way to generate help and usage mesages
use Pod::Usage;

# Used for returning lists of files
use File::Glob;

main();

sub main {
  my $show_manual_page = 0;
  my $show_help_info = 0;

  # The hash %channels is where channel info from cmd line is stored, 
  #    key is channel label (name),
  #    value is the wndchrm signature file associated with that channel
  my %channels;

  my %channel_directories;

  my $top_level_dir = undef;
  my @input_labels = undef;
  my @exclude_labels= undef;
  my $output_label = undef;

  # Two ways to specify combination of sigs:
  # 1. Individually (-c sigfile1.sig=LABEL1 -c sigfile2.sig=LABEL2 -c sigfile3.sig=LABEL3 outfile.sig)
  # 2. En masse ( -d /top/level/dir/ -t TOKEN1 -t TOKEN2 [-t TOKEN3 ... - -o OUTPUT_LABEL )

  GetOptions( 'help|?'  => \$show_help_info,
              man => \$show_manual_page,
              "channel=s" => \%channels,
              "channel_directory|cd=s" => \%channel_directories,
              "top_level_dir=s" => \$top_level_dir,
              "token=s" => \@input_labels,
              "exclude=s" => \@exclude_labels,
              "output_label=s" => \$output_label ) or pod2usage(2);
  pod2usage(1) if $show_help_info;
  pod2usage(-exitstatus => 0, -verbose => 2) if $show_manual_page;

  # Figure out what mode we're in:
  if( %channels && $ARGV[0] ) {
    # Bail out if there aren't at least two channels supplied by user
    if( keys %channels < 2 ) {
      pod2usage("\n********************\nERROR: Must specify at least two channels to combine.\n");
    }
    foreach ( keys( %channels ) ) {
      print "\tSignature file $_ contains channel $channels{$_}\n";
    }
    &WriteCombinedSigFile( { channels => \%channels, outfile => $ARGV[0] } );
  }

  elsif( %channel_directories && $ARGV[0] ) {
    # Bail out if there aren't at least two directories supplied by user
    if( keys %channel_directories < 2 ) {
      pod2usage("\n********************\nERROR: Must specify at least two directories to combine.\n");
    }
    if( ! -d $ARGV[0] ) {
      pod2usage("\n********************\nERROR: $ARGV[0] is not a directory. Please specify a directory for combined signature files to go.\n");
    }
    if( ! -w $ARGV[0] ) {
      die("\n********************\nERROR: Don't have permission to write to directory $ARGV[0]\n");
    }
    my %file_list;
    foreach my $path ( keys( %channel_directories ) ) {
      # Make sure directory exists
      if( ! -e $path ) {
        die "\n********************\nERROR: Path $path does not exist.\n";
      }
      print "\tDirectory $path contains channel $channel_directories{$path}\n";
      my $string_token = $channel_directories{$path};
      my $search_string = "$path/*$string_token*.sig";
      print "\tSearching for $search_string\n";
      @{ $file_list{$string_token} } = < $search_string >;
      # Bail if there were no files in that directory that have that distinguishing string token
      if( $#{ $file_list{ $string_token } } == -1 ) {
        die "\n********************\nERROR: Could not find any .sig files in $path with token $string_token\n";
      }
      #foreach ( @{ $file_list{ $string_token } } ) {
      #  print "$_\n";
      #}
    }
    # Check to make sure there are the same number of sig files in each file list
    my @file_list_keys = keys %file_list;
    my $i;
    for( $i = 0; $i < $#file_list_keys; $i++ ) {
      if( $#{ $file_list{ $file_list_keys[$i] } } != $#{ $file_list{ $file_list_keys[$i+1] } } ) {
        # report the discrepancy across channels, then die
        print "\n***********************\nERROR: Number of signature files is inconsistent across classes:\n";
        for( $i = 0; $i < $#file_list_keys; $i++ ) {
          print "Channel $file_list_keys[$i] has $#{ $file_list{ $file_list_keys[$i] } } signature files.\n";
        }
        die "\nPlease correct the discrepancy and rerun.\n";
      }
    }
    &ProcessFileList( \%file_list, $ARGV[0] );
  }
  else {
    pod2usage("\nNo action taken. Could be an error in your command line arguments. Did you specify an output file/directory?\n");
  }
}
#elsif( defined $top_level_dir && defined %input_labels && $output_label ) {
# Bail out if there aren't at least two labels supplied by user
#  if( keys %input_labels < 2 ) {
#    pod2usage("\n********************\nERROR: Must specify at least two labels to combine.\n");
#  }
#  foreach ( keys( %linput_labels ) ) {
#    print "\tI will search for files with flag \"$_\" in filename for channel 1.\n";
#  }
#  # Check to see if top level directory supplied actually exists
#  if( ! -e $top_level_dir ) {
#    pod2usage("\n********************\nERROR: directory $top_level_dir does not exist!\n" );
#  }
#}

###############################################################
# 

sub ProcessFileList (\%$){
	my ($file_list_ref, $output_dir ) = @_;
  if( !defined $file_list_ref || !defined $output_dir ) {
    die "Process aborted: internal error processing file list.\n";
  }
  my @file_list_string_tokens = keys %{ $file_list_ref };
  my $num_sigs = $#{ $file_list_ref->{ $file_list_string_tokens[0]} };
  my ( $filename_a, $filename_b );
  my $j;
  for( $j = 0; $j < $num_sigs; $j++) {
    # Let's assume, that since all files are ordered by the same algorithm
    # that we don't have to go through the list and identify corresponding
    # sig files. Just shift 'em off the top, and make sure that other than
    # the string token, their filenames all look the same. Die if not.
    my %channels;
    my $file_name;
    foreach my $string_token ( @file_list_string_tokens ) {
      $file_name = shift @{ $file_list_ref->{$string_token} };
      $channels{ $file_name } = $string_token;
    }
    my @file_names = keys %channels;
    my $output_filename = "";
    for( $j = 0; $j < $#file_names; $j++ ) {
      $filename_a = $file_names[$j];
      $filename_b = $file_names[$j+1];
      $filename_a =~ s/.*\///g; # remove path
      if( $filename_a !~ s/$channels{$file_names[$j]}// ) {
        die "\n*****************\nERROR: string token $channels{$file_names[$j]} does not exist in file $filename_a.\n";
      }
      $filename_b =~ s/.*\///g;
      if( $filename_b !~ s/$channels{$file_names[$j+1]}// ) {
        die "\n*****************\nERROR: string token $channels{$file_names[$j+1]} does not exist in file $filename_b.\n";
      }
      if( $filename_a ne $filename_b ) {
        die "\n*****************\nERROR: The files $file_names[$j] and $file_names[$j+1] do not appear to belong together, because when you subtract their respective string tokens, $channels{$file_names[$j]} and $channels{$file_names[$j+1]} respectively, the filenames don't look the same:\n$filename_a\n$filename_b\nPlease check the complement of signature files in their respective directories to make sure each sig file has a corresponding sig file in the other channel.\n\n";
      }
    }
    &WriteCombinedSigFile( \%channels, "$output_dir/$filename_a" );
  }
}

###############################################################
# Here's the main chunk of code:

sub WriteCombinedSigFile (\%$) {
	my ($channels_hash_ref, $outfile ) = @_;
  if( !defined $channels_hash_ref || !defined $outfile ) {
    die "Process aborted: Internal error writing sigfile.\n";
  } 

  # Step 1: Open the output file:
  open OUT, ">$outfile" or die "\n********************\nERROR: Can't open file for writing $outfile: $!\n";

  # Step 2: Dump the contents of each infile into the outfile
  my $count;
  my $channel_count = 0;
  foreach my $infile ( keys %{ $channels_hash_ref } ) {
    $channel_count++;
    open IN, $infile or die "\n********************\nERROR: Can't open file for reading $infile: $!\n";
    print "\tProcessing file $infile\n";
    $count = 0;
    while( <IN> ) {
      # skip the first two lines of each infile, except when first creating outfile.
      if( ++$count <= 2 ) {
        if( $channel_count == 1 ) {
          print OUT $_;
        } else {
          next;
        }
      } else {
        s/\n$//; # strip off the newline character
        print OUT "$_ $channels_hash_ref->{$infile}\n"; # append the label to the signature name
      }
    }
    close IN;
  }

  close OUT;
}
__END__

=head1 combine_channels.pl

combine_channels.pl - Used to combine two or more WNDCHRM signature files into a single signature file, thus combining the signal obtained from the various image channels.

=head1 SYNOPSIS

To combine channels for a single image or tile:   
combine_channels.pl --channel channel1_signatures.sig=LABEL1 --channel channel2_signatures.sig=LABEL2 [--channel channel3_signatures.sig=LABEL3 ...] output_file


To combine channels for multiple images/tiles all at once:   
combine_channels.pl --channel-directory /path/to/channel1/=LABEL1 --channel-directory /path/to/channel2/=LABEL2 [--channel-directory /path/to/channel3/=LABEL3 ...] /desired/output/dir/ 

Options:
-help - show a brief help message
-man - show the full documentation for using this script

=head1 OPTIONS

=over 8

=item B<-help>

Print a brief help message and exits.

=item B<-man>

Prints the manual page and exits.

=back

=head1 DESCRIPTION

B<This program> will read the given input file(s) and do something
useful with the contents thereof.

The second way to create combined channels is to convert many of them at once. You do this by using the --channel-directory argument (--cd does the same thing), followed by the path to the directory where the channel's signature files are, followed by an equal sign '=', followed by the channel's file name "string token, as explained below. Examples:

./combine_channels.pl --channel-directory /path/to/channel_A=CHANNEL_A_STRING_TOKEN --channel-directory /path/to/channel_B=CHANNEL_B_STRING_TOKEN /output/dir/
./combine_channels.pl --cd /home/user/sig_files/hematoxylin_channel/=H1 --cd /home/user/sig_files/eosin_channel/=E1 /home/user/output/combined_sig_files/

where "string token" is the character or characters that, based on your image naming convention, appears somewhere in signature file name identifying the channel to which it belongs. For example, the two signature files, image_12345_HEMATOXYLIN_0_0.sig and image_12345_EOSIN_0_0.sig, have the tokens "HEMATOXYLIN" and "EOSIN" as their tokens.

-dspecifying the directories into which the signature files for the various channels have been separated, fo using the command line arguments "-d /path/to/directory".

When using the "-d /path/to/directory/" option, you must also use the "-t TOKEN" command line arguments for each channel to specify a unique string token that is embedded in the filenames to distinguish which signature files belongs to which channel. For example, if your file naming convention is "img####_hematoxylin.tif" for hematoxylin stain channel, you would use the arguments "-t hematoxylin".

You can also explicitly exclude files that contain a certain string token by using the -e (--exclude) argument.
 
B<Important!> This script will explicity reject any sig file that contains a combination of the labels, for example, if -l _H and -l _E are used as file identifier tokens 

=cut

