#!/usr/bin/env perl

use strict;
use warnings;

use Socket;

my $socket_path = $ARGV[0];
my $buffer;
my $length = 4096;
my $data;

if(not defined $socket_path or length($socket_path) <= 0) {
    print("Usage: $0 /path/to/socket\n");
    exit(1);
}

socket(my $sock, PF_UNIX, SOCK_STREAM, 0) or die "socket: $!";
connect($sock, sockaddr_un($socket_path)) or die "connect: $!";

# do things here
# netstring_encode encodes a single netstring
# netargs_encode encodes an array of arguments
# send($sock,"content",0) to send
# recv($sock,$buffer,$length,0) to receive


send($sock,netargs_encode("bash","-c",'ls -l $HOME').netstring_encode(""),0);
while(recv($sock,$buffer,$length,0))
{
    $data .= $buffer;
}

print("=== data from server ===\n");
print("$data\n");

close($sock);
exit(0);


sub netstring_encode {
    my $string = shift;
    return sprintf("%d:%s,",length($string),$string);
}

sub netargs_encode {
    my @args = @_;
    my $argc = @args;
    return sprintf("%d:%s,%s",length($argc),$argc,join('',map{netstring_encode($_)} @args));
}
