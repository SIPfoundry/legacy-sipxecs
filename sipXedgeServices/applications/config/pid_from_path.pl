#!/usr/bin/perl -w
 
use strict;
use warnings;
 
my @pid_list;
my $status;
 
die ("Usage: $ARGV[0] process_path\n") if (scalar (@ARGV) != 1);
my $target_name = $ARGV[0];
opendir (my $proc_dir, "/proc/") or die ("Cannot open \"/proc/\"\n");
 
while (my $pid = readdir ($proc_dir))
{
  if ($pid =~ m/[0-9]+/) #If an integer
  {
    $status = open (my $comm_file, "/proc/$pid/cmdline");

    if ($status)
    {
      my $command_name = <$comm_file>;
      if ($command_name && $target_name)
      {
        my $pathlen = length($target_name);
        my $command_len = length($command_name);
        if ($command_len && $command_len > $pathlen)
        {
          my $targetpath =  substr($command_name, 0, $pathlen);
          push (@pid_list, $pid) if ($targetpath eq $target_name);
        }
      }
      close($comm_file);
    }
  }
}
closedir ($proc_dir);
 
print (join (" ", @pid_list), "\n");
