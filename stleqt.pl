#!/usr/bin/perl

use strict;
use warnings;
use Time::Piece;

my $futures = (
	"NAME",
	"MONTH", # expiration month as MMMYY
	"OPEN",
	"HIGH",
	"LOW",
	"LAST",
	"SETT",
	"CHGE",
	"VOL",
	"PRIOR_VOL",
	"PRIOR_SETT",
	"PRIOR_INT"
);

#0000000000111111111122222222223333333333444444444455555555556666666666
#0123456789012345678901234567890123456789012345678901234567890123456789
#                PRELIMINARY PRICES AS OF 11/09/17 03:45 PM (CST)
my ($junk, $date) = unpack("A41A18", <>);
my $datetime = Time::Piece->strptime($date, "%D %l:%M %p"); # really CST
#print ">$date<\n";
#print $datetime->strftime("%c %Z"), "\n";
#exit;
while (<>) {
	next if /^TOTAL/;
	next if /^\*\*\*/;
	if (76 > length) {
		#print;
		my $NAME = chop;
	}
	if (87 == length) {
		if (!/^\d/) { # FUTURES
			print;
#0000000000111111111122222222223333333333444444444455555555556666666666
#0123456789012345678901234567890123456789012345678901234567890123456789
#MAR18   1473.20   1485.80B  1462.80A  1478.20A  1478.50   -5.00         245    1483.50          45         224
			my ($month, $open, $openba) = unpack("A5A10A1");
			print "$month|$open|\n";
		}
		else { # OPTION
		}
	}
}
