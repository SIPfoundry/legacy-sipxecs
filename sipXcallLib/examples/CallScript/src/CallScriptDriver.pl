#! /bin/perl

# Driver for CallScript.

# To get WNOHANG for waitpid.
use POSIX ":sys_wait_h";

# Initialize configuration.

# Number of seconds to run before no longer starting any jobs.
$time_limit = 24 * 60 * 60;     # 24 hours
# Number of jobs to attempt to run.
$job_limit = 1;
# Assign SIP listening ports.
$port_base = 10117;
$port_increment = 10;
# Assign RTP send/receive ports.
$rtp_base = 29110;
$rtp_increment = 10;
# Name of subjob program.
$CallScript_program = 'CallScript';
# Do an effective 'pause random $randomize_time' at the start of each job.
$randomize_time = 0;
# Minimum time a subjob must wait to execute, so as to ensure that the parent
# process has recorded it before its SIGCHLD comes in.
$fork_sleep = 2;
# Maximum times to try to select a script that is not already running.
$script_select_tries = 1000;
# 1 if a script should not be selected for a subjob if another subjob
# is already running it.
$no_duplicates = 0;
# 1 if subjob N should always run script N.
$nonrandom = 0;

if ($ARGV[0] eq '-d') {
    $debug = 1;
    shift @ARGV;
}

# Process the arguments, which are either file names, commands, or
# variable assignments..
$errors_found = 0;
%variables = ();
for ($i = 0; $i <= $#ARGV; $i++) {
    $arg = $ARGV[$i];
    # We distinguish files from commands via a hueristic:  All
    # commands start with a verb which does not contain "/" and contain
    # a space.  All files can be put into a form which violates this
    # rule:  relative file names can have "./" prepended, and absolute
    # file names start with "/".  Few file names contain a space.
    # Similarly, we distinguish variable assignments by having an "=" before
    # a "/" or space.
    if ($arg =~ m%^([^/ =]+)=(.*)$%) {
        # Argument looks like a variable assignment
        $variables{$1} = $2;
    } elsif ($arg =~ m%^[^/ ]+ %) {
        # Argument looks like a command.
        # The user's argument 1 is $ARGV[0].
        &process_command($arg, "argument ".($i+1));
    } else {
        # Read the contents of the control file.
        open(C, $arg) ||
            die "Error opening file '$arg' for input: $!\n";
        while (<C>) {
            &process_command($_, "$arg line $.");
        }
        close C;
    }
}
die "$errors_found errors found in commands.\n" if $errors_found > 0;

# Set up the list of scripts depending on whether 'nonrandom' was specified.
if (!$nonrandom) {
    # Set up "cut points" on a scale of 0 to 1, so that a uniform random number
    # in that range will fall into interval $i with the probability that we
    # should select script $i to run.

    $total_weight = 0;
    for ($i = 0; $i <= $#script_weights; $i++) {
        $total_weight += $script_weights[$i];
    }
    die "Total weight is not positive: $total_weight\n" if $total_weight <= 0;
    $cut_point = 0;
    for ($i = 0; $i < $#script_weights; $i++) {
        $cut_point += $script_weights[$i] / $total_weight;
        push(@script_cuts, $cut_point);
    }
    # Set last cut point to exactly 1.
    push(@script_cuts, 1.0);
    if ($debug) {
        print "\@script_files = \n";
        for ($i = 0; $i <= $#script_files; $i++) {
            print "\t$script_files[$i] ", join(' ', @{$script_args[$i]}),
            "\n";
        }
        print "\@script_weights = ", join(' ', @script_weights), "\n";
        print "\@script_cuts = ", join(' ', @script_cuts), "\n";
    }
} else {
    # Duplicate each script according to its weight value.

    my(@script_files_old, @script_args_old);
    @script_files_old = @script_files;
    @script_files = ();
    @script_args_old = @script_args;
    @script_args = ();

    for ($i = 0; $i <= $#script_files_old; $i++) {
        push(@script_files, ($script_files_old[$i]) x $script_weights[$i]);
        push(@script_args, ($script_args_old[$i]) x $script_weights[$i]);
    }
    if ($debug) {
        print "\@script_files = \n";
        for ($i = 0; $i <= $#script_files; $i++) {
            print "\t$script_files[$i] ", join(' ', @{$script_args[$i]}),
            "\n";
        }
    }
    # Ensure there are enough scripts for the number of jobs.
    die "Number of jobs is $job_limit but total script weights is only ",
        $#script_files + 1, ".\n"
            if $job_limit > $#script_files + 1;
}

# Print the PID so the user can easily kill us.
print STDERR "Process ID is $$\n";
print STDERR "Started at ", `date`;

# Run the subordinate scripts.

$start_time = time;
$end_time = $start_time + $time_limit;
print "\$start_time = $start_time, \$end_time = $end_time\n" if $debug;

# Initialize $jobs_running, which is maintained by &start_job and
# &signal_handler_CHLD.
$jobs_running = 0;

# Initialize the array of which scripts each job is running.

for ($i = 0; $i < $job_limit; $i++) {
    $job_script_no[$i] = -1;
}

# Set up interrupt handler for SIGCHLD, which reaps the terminated
# script and possibly starts another.
$SIG{'CHLD'} = \&signal_handler_CHLD;

# Set up interrupt handler for SIGHUP, which shuts down the run gracefully.
$SIG{'HUP'} = \&signal_handler_HUP;

# Set up interrupt handler for SIGQUIT, which shuts down the run
# immediately, since it does not wait for the jobs to finish.
$SIG{'QUIT'} = \&signal_handler_QUIT;

$restart_jobs = 1;

# Start a job for each slot.
for ($i = 0; $i < $job_limit; $i++) {
    &start_job($i);
}

# Wait until time is up.
until (time >= $end_time) {
    select(undef, undef, undef, $end_time - time);
    print "time = ", time, ", \$end_time = $end_time, \$jobs_running = $jobs_running, remaining = ",
          $end_time - time, "\n" if $debug;
}

print "Ending run after ", time - $start_time, " seconds at ", `date`;

# Stop restarting jobs.
$restart_jobs = 0;

# Wait until all running jobs have exited.
until ($jobs_running <= 0) {
    select(undef, undef, undef, 1_000_000);
    print "time = ", time, ", \$end_time = $end_time, \$jobs_running = $jobs_running\n" if $debug;
}

print "Done after ", time - $start_time, " seconds at ", `date`;

# Print the summary.
print "\n";
print "No  Count  Script\n";
$total = 0;
for ($i = 0; $i <= $#script_counts; $i++) {
    printf "%2d  %5d  %s\n", $i, $script_counts[$i], $script_files[$i];
    $total += $script_counts[$i];
}
printf "    %5d  TOTAL\n", $total;

exit 0;

# Signal handler for SIGCHLD.
sub signal_handler_CHLD {
    my($child_pid, $i);

    # Call wait() to get the PID of the child that died.
    # Loop as sometimes the kernel does not deliver exactly as many
    # SIGCHLDs as there are child processes.
    while (($child_pid = waitpid(-1, WNOHANG)) > 0) {
        print "\$child_pid = $child_pid\n" if $debug;

        # Decrement count of jobs currently running.
        $jobs_running--;

        # Remove the job from the table.
        $i = &reap_job($child_pid);

        # Start a job to replace it if we should.
        if ($restart_jobs) {
            &start_job($i);
        }
    }
}

# Start a job in slot $slot.
sub start_job {
    my($slot) = @_;
    my($r, $script_no, $x, $sip_port, $rtp_port, $random_time, $i, @command);
    print "\&start_job($slot)\n" if $debug;

    # Choose the randomization time in the parent process, so the
    # children don't all choose the same time.
    if ($randomize_time > 0) {
        $random_time = int(rand() * $randomize_time / 1000);
    }

    # Determine which script to use.
    # Record in $job_script_no[$slot] which script file number.

    if (!$nonrandom) {
        # Select a script randomly.
      find_script: for ($tries = 0; $tries < $script_select_tries; $tries++) {
          # Select a random number.
          $r = rand();
          # Look it up in the list of cut points.
          for ($script_no = 0; $script_no <= $#script_weights; $script_no++) {
              if ($r <= $script_cuts[$script_no]) {
                  # $script_no is the script selected by the random number.
                  # Check if we should not run duplicates, and if so, check if
                  # this is a duplicate of a script now running.
                  if ($no_duplicates) {
                      for ($i = 0; $i < $job_limit; $i++) {
                          if ($job_script_no[$i] == $script_no) {
                              # $script_no is the script job $i is running, so
                              # try again.
                              next find_script;
                          }
                      }
                  }
                  # We are allowed to run this script.
                  last find_script;
              }
          }
      }
      # Check to see if we exited due to not being able to find a script too
      # many times.
      if ($tries == $script_select_tries) {
          print STDERR "Unable to find a script to execute for job $slot.\n";
          return;
      }
    } else {
        # Select a script deterministically.
        $script_no = $slot;
    }
    # Record that this job is using this script.
    $job_script_no[$slot] = $script_no;

    # Increment the count of jobs running first, to avoid a race condition
    # if it dies between the fork and the time we would increment it.
    $jobs_running++;

    # Fork a subprocess.
    $x = fork();
    if (!defined($x)) {
        print "fork() failed: $!\n";
        # Decrement the count, since we failed to start the job.
        $jobs_running--;
    } elsif ($x == 0) {
        # This is the child process.

        # Select ports to use.
        $sip_port = $port_base + $port_increment * $slot;
        $rtp_port = $rtp_base + $rtp_increment * $slot;

        # Assemble the command to be executed.
        @command = ($CallScript_program, '-p', $sip_port, '-r', $rtp_port,
                    '-l', "Job $slot: ",
                    @{$script_args[$job_script_no[$slot]]});
        # Print the command we will execute now, so the print isn't affected
        # by output redirection.
        print "exec(", join(' ', map { "'$_'" } @command), ")\n"
            if $debug;

        # Sleep the appropriate time.
        sleep $fork_sleep;
        if ($randomize_time > 0) {
            print "Job $slot: pause random $randomize_time (pause ",
                  $random_time*1000, ")*\n";
            sleep $random_time;
        }

        # Set up stdin and stdout.
        open(STDOUT, ">/dev/null") ||
            die "Error opening /dev/null for output: $!\n";
        open(STDIN, "<$script_files[$job_script_no[$slot]]") ||
            die "Error opening '$script_files[$job_script_no[$slot]]' for input: $!\n";
        # Route stderr from subjobs to stderr here (as opposed to routing it
        # to stdout as we used to), so errors from the subjobs are easy to
        # see.

        exec(@command);
        die "Error from exec(" . join(' ', map { "'$_'" } @command) .
            "): $!\n";
    }
    # This is the parent process.
    $subjob_pid[$slot] = $x;
    print "Job $slot: Starting '$script_files[$job_script_no[$slot]]'\n";
    $script_counts[$script_no]++;
    print "\$subjob_pid[$slot] = $x\n" if $debug;
}

# Remove from the table the subjob with the pid $pid.
# Return the slot number.
sub reap_job {
    my($pid) = @_;
    my($i);
    print "\&reap_job($pid)\n" if $debug;

    for ($i = 0; $i <= $job_limit; $i++) {
        if ($pid == $subjob_pid[$i]) {
            print "Job $i: Ending\n";
            $subjob_pid[$i] = 0;
            $job_script_no[$i] = -1;
            print "\$subjob_pid[$i] = 0\n" if $debug;
            print "return $i\n" if $debug;
            return $i;
        }
    }
    die "Cannot find slot of subjob PID $pid.\n";
}

# Set up interrupt handler for SIGHUP, which shuts down the run gracefully.
sub signal_handler_HUP {
    # Set the ending time to now, so the main loop exits.
    print "HUP received.\n";
    $end_time = time;
}

# Set up interrupt handler for SIGQUIT, which shuts down the run
# immediately, since it does not wait for the jobs to finish.
sub signal_handler_QUIT {
    my($i, $pid);

    print "QUIT received.\n";
    # Set the ending time to now, so the main loop exits.
    $end_time = time;
    # Kill all the jobs.
    for ($i = 0; $i <= $job_limit; $i++) {
        if ($subjob_pid[$i] != 0) {
            print "Job $i: Killing\n";
            print "kill -KILL $subjob_pid[$i]\n" if $debug;
            kill('KILL', $subjob_pid[$i]);
        }
    }
    # Set the number of jobs to 0, so the finishing-up loop exits.
    $jobs_running = 0;
}

# Process a single command line.
# $command is the command line/string.
# $id is the identification for error messages.  E.g., "...file... line ..." or
# "argument ...".
sub process_command {
    my($command, $id) = @_;
    my(@tokens, $weight, $file, @args);

    # Substitute for variables.
    chomp $command;
    $command =~
        s/\${([^}]*)}/defined($variables{$1}) ?
                   $variables{$1} :
                   die "No value given for variable '$1' in $id\n"/ge;
    print "\$command = '$command'\n" if $debug;

    # Ignore blank lines.
    return if $command =~ m/^\s*$/;
    # Ignore comments.
    return if $command =~ m/^\s*#/;
    # Get the words of the command.
    @tokens = split(' ', $command);
    print "\@tokens = '", join("' '", @tokens), "'\n" if $debug;
    if ($tokens[0] eq 'time') {
        # time NNN
        $time_limit = $tokens[1];
        # Check its syntax.
        unless ($time_limit =~ /^(\d+)([smhd]?)$/) {
            print STDERR "Invalid time value '$time_limit' in $id\n";
            $errors_found++;
            return;
        }
        # Convert the digit string into a number.
        $time_limit = $1 + 0;
        # Multiply by the right factor.
        # Do nothing for 's' and ''.
        if ($2 eq 'm') {
            $time_limit *= 60;
        } elsif ($2 eq 'h') {
            $time_limit *= 60 * 60;
        } elsif ($2 eq 'd') {
            $time_limit *= 24 * 60 * 60;
        }
        print "\$time_limit = $time_limit\n" if $debug;
    } elsif ($tokens[0] eq 'jobs') {
        # jobs NNN
        $job_limit = $tokens[1] + 0;
        unless ($job_limit > 0) {
            print STDERR "Invalid job limit '$tokens[1]' in $id\n";
            $errors_found++;
            return;
        }
        print "\$job_limit = $job_limit\n" if $debug;
    } elsif ($tokens[0] eq 'program') {
        # program NAME
        $CallScript_program = $tokens[1];
        unless ($CallScript_program ne '' && -x $CallScript_program) {
            print STDERR "Empty or non-executable program name '$CallScript_program' in $id\n";
            $errors_found++;
            return;
        }
        print "\$job_limit = $job_limit\n" if $debug;
    } elsif ($tokens[0] eq 'script') {
        # script NNN FFF
        $weight = $tokens[1] + 0;
        $file = $tokens[2];
        # The following slice appears to work even if $#tokens == 2.
        @args = @tokens[3..$#tokens];
        unless ($weight > 0) {
            print STDERR "Invalid weight '$tokens[1]' in $id\n";
            $errors_found++;
            return;
        }
        unless ($file ne '') {
            print STDERR "Must specify script file in $id\n";
            $errors_found++;
            return;
        }
        unless (-r $file) {
            print STDERR "Cannot read script file '$file' in $id\n";
            $errors_found++;
            return;
        }
        push(@script_weights, $weight);
        push(@script_files, $file);
        push(@script_args, \@args);
        push(@script_counts, 0);
    } elsif ($tokens[0] eq 'randomize') {
        # randomize NNN
        $randomize_time = $tokens[1] + 0;
        unless ($randomize_time >= 0) {
            print STDERR "Invalid randomization time '$tokens[1]' in $id\n";
            $errors_found++;
            return;
        }
        print "\$randomize_time = $randomize_time\n" if $debug;
    } elsif ($tokens[0] eq 'noduplicates') {
        # noduplicates
        $no_duplicates = 1;
        print "\$no_duplicates = $no_duplicates\n" if $debug;
    } elsif ($tokens[0] eq 'nonrandom') {
        # nonrandom
        $nonrandom = 1;
        print "\$nonrandom = $nonrandom\n" if $debug;
    } else {
        printf STDERR "Unrecognized command '$tokens[0]' in $id\n";
        $errors_found++;
        return;
    }
}
