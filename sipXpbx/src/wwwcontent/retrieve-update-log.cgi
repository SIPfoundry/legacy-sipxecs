#!/usr/bin/perl

print "Content-type: application/json\r\n\r\n";
#print "Content-type: text/html\r\n\r\n";

sub parse_form_data
{
  local (*FORM_DATA) = @_;
  local ( $request_method, $query_string, @key_value_pairs, $key_value, $key, $value);
  $request_method = $ENV{'REQUEST_METHOD'};

  if ($request_method eq "GET") {
    $query_string = $ENV{'QUERY_STRING'};
  } else {
    exit(1);
  }

  @key_value_pairs = split (/&/, $query_string);
  foreach $key_value (@key_value_pairs) {
    ($key, $value) = split (/=/, $key_value);
    $value =~ tr/+/ /;
    $value =~ s/%([\dA-Fa-f][\dA-Fa-f])/pack ("C", hex ($1))/eg;
    if (defined($FORM_DATA{$key})) {
      $FORM_DATA{$key} = join ("\0", $FORM_DATA{$key}, $value);
    } else {
      $FORM_DATA{$key} = $value;
    }
  }
}

&parse_form_data (*simple_form);
$start_pos = $simple_form{'start_pos'};
$jsoncallback = $simple_form{'jsoncallback'};

open (FILE, "<" . "/var/log/sipxpbx/update.output.log") || die("Could not open the log file!");
$counter = 1;
$message = "";
while (<FILE>) {
  if ($counter >= $start_pos) {
    chomp;
    $message = $message."$_<br/>";
    #print "$_<br/>";
  }
  $counter++;
}
close (FILE);

$message =~ s/\r/\\n/g;
$message =~ s/"/'/g;

print "$jsoncallback({\"Message\": \"$message\"})";
exit (0);