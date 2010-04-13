#! /bin/perl

# Print the usage message and exit if there are insufficient arguments.
if ($#ARGV < 1) {
    print STDERR <<EOF;
Usage:  $0 log-file call-number call-number ...

One or more numeric call thread numbers must be specified, or
one argument of 'all', to specify all call threads.

Output is into files named <call-number>.{log,events,states}:
    <call-number>.log           all log file lines regarding that call task
    <call-number>.events        call state transitions for that Call-Id
    <call-number>.states        a 1-line file listing all the transitions for that Call-Id

Generates all output files at the same time, so may have a problem
with any limitation on allowed number of open files.
EOF
    exit 1;
}

# Process the arguments.
$log_file = shift @ARGV;
if ($#ARGV == 0 && $ARGV[0] eq 'all') {
    # Select all calls.
    $all_calls = 1;
    %call_interesting = ();
} else {
    # Only specified calls.
    $all_calls = 0;
    foreach $c (@ARGV) {
        die "Bad call number: '$c'\n" unless $c =~ /^\d+$/;
        # Convert $c to a number.
        $c += 0;
        die "Bad call number: '$c'\n" unless $c > 0;
        $call_interesting{$c} = 1;
    }
}

# Variables for dynamically opening files.

# Hash to contain the file handles for all the *.events output files.
# Keyed by call task number.
%file_handle_events = ();
# Hash to contain the file handles for all the *.summary output files.
# Keyed by call task number.
%file_handle_states = ();
# Hash to contain the file handles for all the *.log output files.
# Keyed by call task number.
%file_handle_log = ();

# Number of files dynamically open now.
$files_open = 0;
# Limit after which we should close files.
$max_files_open = 100;

# Get the Call-Id's.
# Read the log file to find the Call-Id's that correspond to the call
# task numbers.
open(LOG, $log_file) ||
    die "Error opening mediaserver log file '$log_file' for input: $!\n";
while (<LOG>) {
    split(/:/);
    # The task name is $_[7].
    if ($_[7] =~ m!^Call-(\d+)$!) {
        # This is a log line for a call task.
        # Check if it is one we care about.
        $call_task_no = $1 + 0;
        if ($all_calls || $call_interesting{$call_task_no}) {
            # Extract the Call-Id: value if there is one.
            # Backslash is an allowed character in a Call-Id, but the logging
            # system quotes backslashes.  This regexp still fails if
            # logged data contains something that looks like a Call-Id: header.
            if ($x = (m!\\r\\n(Call-Id|i)\s*:\s*(([\041-\133\135-\176]|\\)+)\s*\\r\\n!i)[1]) {
                # Check if we already have a Call-Id for this task number,
                # and if so, report it.
                if (defined($call_id{$call_task_no})) {
                    # Report an error if it is different.
                    if ($x ne $call_id{$call_task_no}) {
                        printf STDERR
                            "Multiple Call-Id's found for call $call_task_no: $call_id{$call_task_no}\n";
                        printf STDERR
                            "Multiple Call-Id's found for call $call_task_no: $x\n";
                    }
                }
                # Record this Call-Id for this task number.
                # Record it even if this is a second Call-Id for this task number,
                # because succeeding log lines for this task number are more likely
                # to use the new Call-Id than the old one.
                $call_id{$call_task_no} = $x;
                $call_no{$x} = $call_task_no;
                #printf "Call-ID for call $call_task_no: $call_id{$call_task_no}\n";
            }
        }
    }
}
close LOG;

# Check that we found call ids for all calls of interest.
if (!$all_calls) {
    foreach $c (sort { $a <=> $b } keys %call_interesting) {
        if (!defined($call_id{$c})) {
            printf STDERR "No Call-Id found for call $c.\n";
        }
    }
}

# Get the call state transitions.
open(LOG, $log_file) ||
    die "Error opening mediaserver log file '$log_file' for input: $!\n";
$line_selector = '(mediaserver:"main: getAndClearCallStateLog: |CallManager::logCallState: )';
while (<LOG>) {
    if (m/$line_selector/o) {
        # Process the call state log lines.

        # Split apart the call state transition records
        s/^.*$line_selector\s*(\\n)*//o;
        split(/------*END------*(\\n)?/);
        foreach $x (@_) {
            # Filter out transitions for this call.
            $call_id = ($x =~ (m!\\nCallId: (([\041-\133\135-\176]|\\)+)\s*\\n!))[0];
            $call_task_no = $call_no{$call_id};
            if (defined($call_task_no)) {
                # Format nicely for printing.
                # Remove final \n, if any.
                $x =~ s/\\n$//;
                # Change other \n's into LF-TAB.
                $x =~ s/\\n/\n\t/g;
                # Print the line to the events file.
                # Open the events file if necessary
                if (!defined($file_handle_events{$call_task_no})) {
                    open($file_handle_events{$call_task_no}, ">$call_task_no.events") ||
                        die "Error opening '$call_task_no.events' for output: $!\n";
                }
                print {$file_handle_events{$call_task_no}} $x, "\n";
                # Print the transition to the states file.
                # Open the states file if necessary
                if (!defined($file_handle_states{$call_task_no})) {
                    open($file_handle_states{$call_task_no}, ">$call_task_no.states") ||
                        die "Error opening '$call_task_no.states' for output: $!\n";
                    # Print the call number at the beginning.
                    print {$file_handle_states{$call_task_no}} "$call_task_no\t";
                } else {
                    # Print a / after the last state.
                    print {$file_handle_states{$call_task_no}} " ";
                }
                # Extract the transition name, which contains no colon and is all upper-case.
                $y = ($x =~ m!\n\t([A-Z_]+)\n!)[0];
                print {$file_handle_states{$call_task_no}} $y;
            }
        }
    } elsif (# Select log lines from call tasks, and log lines about
             # calls from SipClient tasks.
             (
              # The task name is $_[7] when the log line is split on ':'..
              (split(/:/))[7] =~ m!^Call-(\d+)$! &&
              ($call_task_no = $2) &&
              ($all_calls || $call_interesting{$call_task_no+0})
             ) || (
              # Try to find a Call-Id, and see if it is one we are interested
              # in..
              m!\\r\\n(Call-Id|i)\s*:\s*(([\041-\133\135-\176]|\\)+)\s*\\r\\n!i &&
              defined($call_task_no = $call_no{$2})
             )) {
        # Process log lines for one of the selected calls.
        # Print the line to the split log file.
        # Open the split log file if necessary
        if (!defined($file_handle_log{$call_task_no})) {
            open($file_handle_log{$call_task_no}, ">$call_task_no.log") ||
                die "Error opening '$call_task_no.log' for output: $!\n";
        }
        print {$file_handle_log{$call_task_no}} $_;
    }
}
close LOG;
# Close all the output files.
foreach $h (values %file_handle_events) {
    close $h;
}
foreach $h (values %file_handle_states) {
    print $h "\n";
    close $h;
}
foreach $h (values %file_handle_log) {
    close $h;
}
