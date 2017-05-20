#!/usr/bin/env perl

# example of sending a shell script via stdin
# also example of sending stdin over multiple sends

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


print("Sending data\n");
send($sock,netargs_encode('bash'),0);
print("Sent, sleeping\n");
sleep(3);

print("Sending data\n");
send($sock,netstring_encode('set -x;echo hello from shell;exit 0'),0);
print("Sent, sleeping\n");
sleep(3);

print("Sending data\n");
send($sock,netstring_encode(''),0);
print("Sent, receiving\n");
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
