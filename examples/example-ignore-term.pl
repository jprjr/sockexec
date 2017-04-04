#!/usr/bin/env perl

# calls a program that ignores TERM signals, waits
# one second, then disconnects.
#
# After disconnecting, the program will will continue
# running until sockexec sends the KILL signal

use strict;
use warnings;

use Socket;
use Cwd 'abs_path';
use File::Basename;

my $me = abs_path($0);
my $ignore_term = dirname($me) . '/ignore-term';

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

send($sock,netargs_encode($ignore_term),0);
send($sock,netstring_encode(""),0);

sleep(1);

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
