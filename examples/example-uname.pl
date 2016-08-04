#!/usr/bin/env perl

# just requests uname of the system running sockexec

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

send($sock,netargs_encode('uname','-a').netstring_encode(""),0);
while(recv($sock,$buffer,$length,0))
{
    $data .= $buffer;
}
close($sock);

print("=== data from server ===\n");
printf("$data\n");

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
