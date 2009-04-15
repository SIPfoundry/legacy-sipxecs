#! /bin/perl

# Program to test the statistical quality of hash functions.

# To set it up, adjust the call "$h = &hash(...)" below to call your
# hash function, and define your hash function.  Adjust the while loop
# to read your sample data items into $_, so they get hashed.

# The current hash function is a simple shift-and-xor scheme, and the
# data items are strings read from <DATA>.

# The program will examine each hash value returned, and tally the
# number of 0 and 1 bits that appear in each bit location, as well as
# the correlation between every pair of bit locations.  All this
# information is printed in a summary table.

# Bit	0s	1s	z	p
# 0	711	705	0.03986	1.00000 
# 1	738	678	0.39862	1.00000 
# ...
# 30	1416	0	9.40744	0.00100 *****
# 31	1416	0	9.40744	0.00100 *****

# This table gives the census of 0 and 1 bits in each location, as
# well as the statistical deviation (z) from the expected 50/50
# distribution, and the two-tailed z-test significance p.  (Actually, p
# is rounded up to the next higher number in the sequence 1, 0.1,
# 0.05, 0.01, 0.005, 0.001.)  An additional star is appended for each
# lower p step a value reaches.

# Bits		0/0	0/1	1/0	1/1	chi2	p
# 1	0	373	365	338	340	0.06720	1.00000 
# 2	0	358	350	353	355	0.07062	1.00000 
# 2	1	361	347	377	331	0.72446	1.00000 
# 3	0	354	364	357	341	0.48062	1.00000 
# 3	1	378	340	360	338	0.16247	1.00000 
# 3	2	347	371	361	337	1.62744	1.00000 
# 4	0	353	362	358	343	0.40884	1.00000 
# 4	1	364	351	374	327	0.84672	1.00000 
# 4	2	369	346	339	362	1.49450	1.00000 
# 4	3	381	334	337	364	3.84774	0.05000 **
# ...
# 24	0	711	705	0	0	
# 24	1	738	678	0	0	
# 24	2	708	708	0	0	
# ...

# This table gives the census of 0 and 1 combinations seen between the
# two bit positions. For positions that are all 0 or all 1, no
# correlation test can be done.  For the rest, the "chi square test
# with 1 degree of freedom" is done, giving chi2 and p.

# Generally, bigger p values are better, and stars are bad.  But you
# *expect* to see p < 0.05 about 1 in 20 times, so some stars are
# inevitable.

$debug = 0;

# Clear the counters.
# Total samples.
$samples = 0;
# Zero and one bits in each position.
for ($i = 0; $i < 32; $i++) {
    ${$freq[$i]}[0] = 0;
    ${$freq[$i]}[1] = 0;
}
# Crosstabulation of zeros and ones in every pair of positions.
for ($i = 0; $i < 32; $i++) {
    for ($j = 0; $j <= $i; $j++) {
        ${${${$corr[$i]}[$j]}[0]}[0] = 0;
        ${${${$corr[$i]}[$j]}[0]}[1] = 0;
        ${${${$corr[$i]}[$j]}[1]}[0] = 0;
        ${${${$corr[$i]}[$j]}[1]}[1] = 0;
    }
}

# Suck in a series of strings to hash
while (<DATA>) {
    chomp $_;
    print "\$_ = '$_'\n" if $debug;
    $h = &hash($_);
    print "\$h = $h\n" if $debug;
    for ($i = 0; $i < 32; $i++) {
	$bit[$i] = $h & 1;
	$h = $h >> 1;
    }
    print join(' ', @bit), "\n" if $debug;
    # Tally the samples.
    $samples++;
    # Tally the bit frequencies.
    for ($i = 0; $i < 32; $i++) {
	${$freq[$i]}[$bit[$i]]++
    }
    # Tally the correlation frequencies.
    for ($i = 0; $i < 32; $i++) {
	for ($j = 0; $j < $i; $j++) {
	    ${${${$corr[$i]}[$j]}[$bit[$i]]}[$bit[$j]]++;
	}
    }
    last if $debug;
}

print "Bit\t0s\t1s\tz\tp\n";
for ($i = 0; $i < 32; $i++) {
    # Calculate the z score.
    # Mean of a bit is 0.5 and s.d. is 0.5, so mean of sum
    # is $samples/2 and s.d. is 0.5/sqrt($samples)
    $z = abs((${freq[$i]}[0] - $samples/2) / (2 * sqrt($samples)));
    ($z_stat, $flag) = &z_stat($z);
    print "$i\t", ${$freq[$i]}[0], "\t", ${$freq[$i]}[1], "\t",
          sprintf("%.5f", $z), "\t", sprintf("%.5f", $z_stat),
          " ", $flag, "\n";
}
print "\n";

print "Bits\t\t0/0\t0/1\t1/0\t1/1\tchi2\tp\n";
for ($i = 0; $i < 32; $i++) {
    for ($j = 0; $j < $i; $j++) {
	$chi2 = &chi2(${${${$corr[$i]}[$j]}[0]}[0],
                      ${${${$corr[$i]}[$j]}[0]}[1],
                      ${${${$corr[$i]}[$j]}[1]}[0],
                      ${${${$corr[$i]}[$j]}[1]}[1]);
	print "$i\t$j\t",
        ${${${$corr[$i]}[$j]}[0]}[0], "\t",
        ${${${$corr[$i]}[$j]}[0]}[1], "\t",
        ${${${$corr[$i]}[$j]}[1]}[0], "\t",
        ${${${$corr[$i]}[$j]}[1]}[1], "\t";
        if ($chi2 > -1) {
	    ($chi2_stat, $flag) = &chi2_stat($chi2);
	    print substr(sprintf("%.5f", $chi2), 0, 7), "\t",
	          sprintf("%.5f", $chi2_stat), " ", $flag;
	}
        printf "\n";
    }
}

exit 0;

# Hash a string using the UtlString algorithm.
sub hash {
    my($s) = @_;
    my($h, $i);

    $h = 0;
    for ($i = 0; $i < length($s); $i++) {
	$h = (($h << 5) - $h + ord(substr($s, $i, 1))) & 0xFFFFFF;
    }
    return $h;
}

# Hash a string using a division algorithm.
sub hash_div {
    my($s) = @_;
    my($h, $i);

    $h = 0;
    for ($i = 0; $i < length($s); $i++) {
	$h = ( ($h << 8) + ord(substr($s, $i, 1)) ) % 0xFFFFFA;
    }
    return $h;
}

# The 32-bit FNV hash, truncated to 24 bits.
sub hash_fnv {
    use integer;
    my($s) = @_;
    my($h, $i);
    my($p) = 0x193;

    $h = 2_166_136_261 & 0xFFFFFF;
    for ($i = 0; $i < length($s); $i++) {
	$h = (($h ^ ord(substr($s, $i, 1))) * $p) & 0xFFFFFF;
    }
    return $h;
}

# Calculate the liklihood of a z-score.
sub z_stat {
    my($z) = @_;
    my($i);
    my(@significance) = (1, 0.1, 0.05, 0.01, 0.005, 0.001);
    my(@values) = (0, 1.645, 1.960, 2.576, 2.807, 3.290);
    my(@flag) = ('', '*', '**', '***', '****', '*****');

    for ($i = $#values; $i >= 0; $i--) {
	if ($z >= $values[$i]) {
	    return ($significance[$i], $flag[$i]);
	}
    }
}

# Calculate the chi2 of a 2x2 array.
sub chi2 {
    my($o00, $o01, $o10, $o11) = @_;
    my($r0, $r1, $c0, $c1, $t, $e00, $e01, $e10, $e11, $chi2);

    $r0 = $o00 + $o01;
    $r1 = $o10 + $o11;
    $c0 = $o00 + $o10;
    $c1 = $o01 + $o11;
    return -1 if $r0 == 0 || $r1 == 0 || $c0 == 0 || $c1 == 0;
    $t = $r0 + $r1;
    $e00 = $r0 * $c0 / $t;
    $e01 = $r0 * $c1 / $t;
    $e10 = $r1 * $c0 / $t;
    $e11 = $r1 * $c1 / $t;
    $chi2 =
	($o00 - $e00) * ($o00 - $e00) / $e00 +
	($o01 - $e01) * ($o01 - $e01) / $e01 +
	($o10 - $e10) * ($o10 - $e10) / $e10 +
	($o11 - $e11) * ($o11 - $e11) / $e11;
    return $chi2;
}

# Calculate the liklihood of a chi2-score with 1 d.f.
sub chi2_stat {
    my($chi2) = @_;
    my($i);
    my(@significance) = (1, 0.1, 0.05, 0.01, 0.005, 0.001);
    my(@values) = (0, 2.706, 3.841, 6.635, 7.879, 10.83);
    my(@flag) = ('', '*', '**', '***', '****', '*****');

    for ($i = $#values; $i >= 0; $i--) {
	if ($chi2 >= $values[$i]) {
	    return ($significance[$i], $flag[$i]);
	}
    }
}

__DATA__
A
A server,
AS
AS IS"
AS IS" basis
Abstract
Acknowledgement
Acknowledgements
Acknowledgements ....................................
Acknowledgements .................................... 14
