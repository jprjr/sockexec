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

send($sock,netargs_encode('tail','-f','/tmp/xx.log').netstring_encode(""),0);
while(recv($sock,$buffer,$length,0))
{
    print("=== data from server ===\n");
    printf("$buffer\n");
}

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

sub netstring_decode {
    my $string = shift;
    if(not defined($string) or length($string) ==0) {
        return undef;
    }

    my $i = 0;
    my $ret = [];
    while($i < length($string)) {
        my $lengthstring = '';
        while(substr($string,$i,1) ne ':') {
            $lengthstring .= substr($string,$i,1);
            $i++;
        }
        if(substr($string,$i+$lengthstring+1,1) ne ',') {
            return undef;
        }
        push(@$ret, substr($string,$i+1,$lengthstring));
        $i = $i + $lengthstring + 2;
    }

    return $ret;
}

sub sockexec_decode {
    my $string = shift;
    my $ret = {};

    my $t = undef;
    my $decoded = netstring_decode($string);
    foreach my $s (@$decoded) {
        if(not defined($t)) {
            $t = $s;
        }
        else {
            if(not exists($ret->{$t})) {
                $ret->{$t} = '';
            }
            $ret->{$t} .= $s;
            $t = undef;
        }
    }
    return $ret;
}
