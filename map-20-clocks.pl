#!/usr/bin/perl

# NOTE: if you run this a few times with libplace.so running, you'll cauze a
# segmentation fault in libplace.so

use common::sense;
use forks;

$SIG{USR1} = $SIG{USR2} = $SIG{TERM} = $SIG{INT} = sub { exit 0 };

my ($max) = @ARGV;
$max //= 20;

my ($x, $y) = (5, 30);
my @clocks = map {
    $x += 75;

    if( $x > 1500 ) {
        $x = 5;
        $y += 80;
    }

    threads->create(sub {
        say "$$ starting up";
        @ARGV = ($x, $y);
        do "./render-clock.pl";
        say "$$ shutting down …";
    })

} 1 .. $max;

$SIG{USR1} = $SIG{USR2} = $SIG{TERM} = $SIG{INT} = sub { say "sure hope they all shut down …" };

$_->join for @clocks;
exit 0;
