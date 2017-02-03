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

print("Sending args\n");
send($sock,netargs_encode('cat'),0);

sleep(1);

print("Sending data\n");
send($sock,netstring_encode('first line'),0);

sleep(1);

print("Getting data\n");
recv($sock,$buffer,$length,0);

print("=== data ===\n");
print("$buffer\n");

print("Sending data\n");
send($sock,netstring_encode('second line'),0);

sleep(1);

print("Closing stdin\n");
send($sock,netstring_encode(''),0);

sleep(1);

print("Getting data\n");

while(recv($sock,$buffer,$length,0))
{
    $data .= $buffer;
}
print("=== data ===\n");
print("$data\n");
#
#send($sock,netstring_encode('{"three";3,"four":4}'),0);
#while(recv($sock,$buffer,$length,0))
#{
#    $data .= $buffer;
#}
#print("=== data ===\n");
#print("$data\n");
#
#send($sock,netstring_encode(""),0);
#
#while(recv($sock,$buffer,$length,0))
#{
#    $data .= $buffer;
#}
#close($sock);
#
#print("=== data from server ===\n");
#print("$data\n");

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
