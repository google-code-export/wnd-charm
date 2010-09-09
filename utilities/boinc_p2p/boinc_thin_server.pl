#!/usr/bin/perl -w

# change include path for modules to include current directory
BEGIN{ push @INC, "./"; }

use strict;
use warnings;

use BoincDaemon; # A modded version of HTTP::Daemon
use HTTP::Status;

my $counter = 0;

# Ports 0-1024 are reserved by the OS, therefore you'll have to
# run this code as root to listen on port 80.
my $daemon = BoincDaemon->new( LocalPort => 31416 ); 
die "unable to new daemon\n" unless($daemon);

# print the name that this server will use to identify itself
print $daemon->product_tokens . "\n";
print "Filehandle: " . $daemon->fileno . "\n";
print "URL:" . $daemon->url . "\n";

our $connection; # for the socket

# Redefine what happens when this script receives a terminate signal
# Be diligent about closing any open socket.
$SIG{'TERM'} = sub { if ($connection) { $connection->shutdown(2); $connection->close; }};
$SIG{'__DIE__'} = sub { if ($connection) { $connection->shutdown(2); $connection->close; } };

# accept() method returns when connection from client is available
# $connection is an instance of the HTTP::Daemon::ClientConn
while( $connection = $daemon->accept )
{
  print "A connection was accepted...\n";
  # $request is an instance of HTTP::Request
  my $request = $connection->get_request;
  if( $request )
  {
    #print $request->as_string, "\n";
    my $path = $request->url->path;
    my $url = $request->uri;
    my $method = $request->method;
    my $qstring = $request->uri->query; 
    my $content = ${ $request->content_ref }; 
    my $iaddr = $connection->peeraddr ; 
    my $peer = gethostbyaddr($iaddr, AF_INET);
    print "handed request: $path and $url \n"; 
    print " we got content <$content>\n" if($content); 
    print " we got qstring <$qstring>\n" if($qstring);
   
    if ($method eq 'GET' && $path =~ /^\/$/ )
    { 
    print "Sending response...\n";
    my $h = HTTP::Headers->new;

    $h->header('Content-Type' => 'text/plain');
    $counter++;
    my $response = HTTP::Response->new( 200, "OK", $h, "HELLO!!!" . $counter );
    $connection->send_response( $response );
    #  $c->send_file_response("hello.html"); 
    } 
    elsif ( -f $path )
    { 
      $connection->send_file_response( $path ); 
    } 
    elsif ( -f "../cgi_bin${path}" )
    { 
      # # we know it is something we can invoke
      # let us just cope with the game now
      #handle_request($r, $c);
      print "Sorry, no cgi.\n";
    } 
    else 
    { 
      $connection->send_error(RC_NOT_FOUND) 
    } 
    undef($request);
   
  }
  else
  {
    print "Didn't work: ". $connection->reason . "\n";
  }

  $connection->shutdown(2);
  $connection->close;
  undef($connection);
}


