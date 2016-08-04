#!/usr/bin/env perl

# asks sockexec to call 'sleep' for 90 seconds
# should receive a termsig event instead of
# an exitcode event, assuming sockexec is
# running with the default 60-sec timeout

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

send($sock,netargs_encode('sleep','90'),0);
send($sock,netstring_encode(""),0);

while(recv($sock,$buffer,$length,0))
{
    $data .= $buffer;
}
close($sock);


print("=== data from server ===\n");
print("$data\n");

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
