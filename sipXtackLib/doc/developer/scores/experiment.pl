#! /usr/bin/perl

$trials = 10_000;

@weights = (10, 1, 1, 10);

# Generate $trials random orders and count them.
for ($i = 0; $i < $trials; $i++) {
#    print "A $i\n";
    @array =
	map { [ $_, $weights[$_], - log(rand()) / $weights[$_] ] }
        (0..$#weights);
#    for ($j = 0; $j <= $#array; $j++) {
#	print "B ${$array[$j]}[0] ${$array[$j]}[1] ${$array[$j]}[2]\n";
#    }
    @array = sort c @array;
#    for ($j = 0; $j <= $#array; $j++) {
#	print "C ${$array[$j]}[0] ${$array[$j]}[1] ${$array[$j]}[2]\n";
#    }
    $order = '';
    map { $order .= ${$_}[0] } @array;
#    print "D $order\n";
    $count{$order}++;
}

# Go through all the orders seen and account for them against their
# preceding orders.
foreach $k (sort(keys(%count))) {
    if (!defined($parent{$k})) {
	&calc_stats($k);
    }
}

# Print the parameters.
print "Weights:\n";
for ($i = 0; $i <= $#weights; $i++) {
    print "$i\t$weights[$i]\n";
}
print "\n";

print "Trials: $trials\n";
print "\n";

# Print the number of occurrences.
print "Order\tCount\n";
foreach $k (sort(keys(%count))) {
    print "$k\t$count{$k}\n";
}
print "\n";

# Print the status for every order.
print "Order\tCount\tParent\tP.Count\tTheory\tObserved\n";
foreach $k (sort(keys(%parent))) {
    if ($k ne '') {
	print "$k\t$partial_count{$k}\t$parent{$k}\t$partial_count{$parent{$k}}\t";
	print sprintf("%.5f\t", $ratio{$k});
	print sprintf("%.5f\t", $partial_count{$k}/$partial_count{$parent{$k}});
        print "\n";
    }
}

exit 0;

sub c {
    $$a[2] <=> $$b[2];
}

# Calculate statistics for each partial ordering.
sub calc_stats {
    my($key) = @_;
    my($count, $k, $parent, $w, $i);

    # Get the count of the times this partial ordering was seen.
    $count = 0;
    foreach $k (keys(%count)) {
	if ($key eq substr($k, 0, length($key))) {
	    $count += $count{$k};
	}
    }
    # Get the parent partial ordering.
    if ($key eq '') {
	$parent = '';
    } else {
	$parent = substr($key, 0, length($key)-1);
	# Get its stats, if not already calculated.
	if (!defined($parent{$parent})) {
	    &calc_stats($parent);
	}
    }

    # Calculate the theoretical ratio.
    # Get all the weights not in the parent.
    $w = 0;
    for ($i = 0; $i <= $#weights; $i++) {
	if (index($parent, $i) == -1) {
	    $w += $weights[$i];
	}
    }
    $ratio = $weights[substr($key, -1)] / $w;

    $parent{$key} = $parent;
    $partial_count{$key} = $count;
    $ratio{$key} = $ratio;
}
