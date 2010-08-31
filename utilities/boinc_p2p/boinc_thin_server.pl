#!/usr/bin/perl
BEGIN{ push @INC, "./"; }
use strict;
use warnings;

use BoincDaemon;
use HTTP::Status;

my $counter = 0;
# Ports 0-1024 are reserved by the OS, therefore you'll have to
# run this code as root to listen on port 80.
my $daemon = BoincDaemon->new( LocalPort => 31416 ) || die;

# print the name that this server will use to identify itself
print $daemon->product_tokens, "\n";
print "Please contact me at: <URL:", $daemon->url, ">\n";

# accept() method returns when connection from client is available
# $connection is an instance of the HTTP::Daemon::ClientConn
while( my $connection = $daemon->accept )
{
  print "Something happened...\n";
  # $request is an instance of HTTP::Request
  my $request = $connection->get_request;
  if( $request )
  {
    print $request->as_string, "\n";
    print "Sending response...\n";
    my $h = HTTP::Headers->new;

    $h->header('Content-Type' => 'text/plain');
    $counter++;
    my $response = HTTP::Response->new( 200, "OK", $h, "HELLO!!!" . $counter );
    $connection->send_response( $response );
  }
  else
  {
    print "Didn't work: ". $connection->reason . "\n";
  }

  $connection->close;
  undef($connection);
}


