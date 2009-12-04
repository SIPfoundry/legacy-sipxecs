#! /usr/bin/perl

use CGI qw/:standard/;
BEGIN
{

    $RequireMsg = <<REQUIREMENTS;

    You must install the missing Perl package(s) to use this script.
        As far as I know, these are not available as rpms.  You can install
        them using 'cpan'.

REQUIREMENTS

        @modules = qw(Getopt::Long XML::Simple XML::Parser File::ReadBackwards);
    for $pm ( @modules )
    {
        eval "require $pm;" || die "Failed to load $pm\n$@\n$RequireMsg\n";
        $pm->import();
    }
}
use strict;

# The un-escape table for backslash escapes.
my(%unescape) = ("r", "\r",
                 "n", "\n",
                 "\"", "\"",
                 "\\", "\\");

# Read and parse the registrations file.

my($registration_file) = "@SIPX_DBDIR@/registration.xml";

# Read and process the registrar log files.
my(%user_agent);
my(%need_user_agent);

my @registrations;
if (-e $registration_file) {

    # Read and parse the registrations file.
    my($parser) = new XML::Parser(Style => 'Tree');
    my($tree) = $parser->parsefile($registration_file);

    # Extract the information about the registrations.
    if ($tree->[0] eq 'items') {
        my $c = $tree->[1];
        my $i;
        my($timestamp) = time;
        for ($i = 1; $i < $#$c; $i += 2) {
            if ($c->[$i] eq 'item') {
                my($d) = $c->[$i+1];
                my($i);
                # Create the hash to contain the information about this
                # registration.
                my($registration) = {};
                for ($i = 1; $i < $#$d; $i += 2) {
                    my($e) = $d->[$i];
                    my($f) = $d->[$i+1];
                    if ($e eq 'callid') {
              $registration->{'callid'} = &text($f);
              $need_user_agent{$registration->{'callid'}}=1;
                    } elsif ($e eq 'cseq') {
                        $registration->{'cseq'} = &text($f);
                    } elsif ($e eq 'uri') {
                        $registration->{'AOR'} = &text($f);
                    } elsif ($e eq 'contact') {
                        $registration->{'contact'} = &text($f);
                    } elsif ($e eq 'qvalue') {
                        $registration->{'q'} = &text($f);
                    } elsif ($e eq 'expires') {
                        $registration->{'expires'} = (&text($f) - $timestamp) . '';
                    } elsif ($e eq 'instance_id') {
                        $registration->{'instance_id'} = &text($f);
                    } elsif ($e eq 'gruu') {
                        $registration->{'gruu'} = &text($f);
                    }
                }
                # Add to the array of registrations.
                push(@registrations, $registration);
            }
        }
    } else {
        # Crash if the registrations file is unparsable.
        exit 1;
    }

}

&findUserAgentValues; # reads log backwards, until each %need_user_agent, is in %user_agent

# Generate the table body in order by extension.
my($table_body) = '';
my($registration);
foreach $registration (sort registration_cmp @registrations) {
    # Ignore registrations that have expired.
    if ($registration->{'expires'} > 0) {
        my($AOR) = $registration->{'AOR'};
        my($extension) = $AOR =~ /sips?:(\d+)@/;
        $table_body .=
            &Tr({ style => 'border: 2px solid; border-bottom-width: 1px'},
                &th({-rowspan => 2, -align => 'left'},
                    [$extension,
                     &show($AOR, 1),
                     ]),
                &td({-align => 'right', style => 'border-bottom-width: 1px'},
                    ( $registration->{'q'}
                      ? &show($registration->{'q'}, 0)
                      : '<i>1.0</i>'
                      ),
                    ),
                &td({-align => 'right', style => 'border-bottom-width: 1px'},
                    &show($registration->{'contact'}, 1)
                    ),
                &td(
                    {style => 'border-bottom-width: 1px',
                     colspan => ($registration->{'instance_id'} ? '1' : '2')
                     },
                    ($registration->{'instance_id'}
                     ? [
                        &show($registration->{'instance_id'}, 1),
                        &show('sip:' . $registration->{'gruu'}, 1)
                        ]
                     : '<font color="red">+sip.instance parameter required for gruu; none seen</font>'

                     )),
                ) . "\n";;
        $table_body .=
            &Tr({style => 'border: 2px solid; border-top-width: 1px'},
                &td({-align => 'right'},
                    &show($registration->{'cseq'}, 0)
                    ),
                &td(&show($registration->{'callid'}, 1)),
                &td(&relativeTime($registration->{'expires'})),
                &td(&show($user_agent{$registration->{'callid'}}, 1))
                ) . "\n";
    }
}

# Start the HTML.
print &header,
    &start_html('Registrations @ ' . server_name()), "\n",
    &h1(' Registrations @ ', &a({-href=>"/"}, server_name())), "\n";

# Print SIPit rules warning.
print hr();
print blockquote(
   'Any information you obtain about other users of this server you may use only under the ',
    a({href=>"https://www.sipit.net/FirstTimers"},"SIPit rules"),
    '.'
    );
print hr();

# Beware that <tr> is generated by the Tr() function, not the tr() function,
# because 'tr' is a keyword in Perl.
print &p(&table({-border => 1, -style => 'border-collapse: collapse; border: 2px solid', -cellpadding => '5'},
                &Tr({ -style => 'border: 2px solid;border-bottom-width: 1px;'},
                    &th({-rowspan => 2},
                        [
                         'Ext.',
                         'AOR'
                         ]),
                    &th(
                        [
                         'q',
                         'Contact',
                         'Instance ID',
                         'GRUU',
                         ]),
                    ),
                &Tr({ -style => 'border: 2px solid; border-top-width: 1px'},
                    &th([
                         'CSeq',
                         'Call-Id',
                         'Expires',
                         'User-Agent',
                         ])
                    ), "\n",
                $table_body)),
    "\n";
my($d) = `date -u '+%Y-%m-%dT%H:%M:%SZ'`;
chomp($d);
print &p("Data taken from $registration_file at $d.");

# Print the boilerplate.
print <<EOF;
<h2>Fields</h2>
<dl>
<dt>AOR</dt><dd>is the address of record for which the contact is registered.</dd>
<dt>Contact</dt><dd>is the contact address which is registered.
The <code>x-sipX-nonat</code> and <code>x-sipX-pubcontact</code> URI parameters
are added by sipX's NAT traversal support.</dd>
<dt>User-Agent</dt><dd>is the value from the User-Agent header in the REGISTER.</dd>
<dt>q</dt><dd>is the q-value which shows the preference of this contact for this AOR
(1.0 = highest, 0.0 = lowest); a value shown in italics (<i>1.0</i>) means that no q parameter was supplied and the value is implicit.</dd>
<dt>Expires</dt><dd>is the number of seconds until this registration expires.</dd>
<dt>Instance ID</dt><dd>is the <code>+sip.instance</code> given for this contact.
<dt>GRUU</dt><dd>is the GRUU that was assigned for this contact.
    A GRUU will only be assigned if a <code>+sip.instance</code> was provided.</dd>
<dt>Call-Id</dt><dd>is the Call-Id of the REGISTER request that established this
    registration.
    Multiple registrations for the same contact with different
    Call-Id values show that the UA is incorrectly using different Call-Id values
    for successive REGISTERs.</dd>
<dt>CSeq</dt><dd>is the CSeq number of the REGISTER.
    A <font color='red'>red</font> value means that the UA has incorrectly
    sent more than 2 registrations in the last 5 minutes, which means that
    it is re-registering too frequently.</dd>
</dl>
EOF

# End the HTML.
print &end_html,
    "\n";

exit 0;

# Extract the (top-level) text content from an XML tree.
sub text {
    my($tree) = @_;
    my($text) = '';
    my $i;
    for ($i = 1; $i < $#$tree; $i += 2) {
        if (${$tree}[$i] eq '0') {
            $text .= ${$tree}[$i+1];
        }
    }
    return $text;
}

# Function to compare two registration hashes.
sub registration_cmp {
    # Extract the AOR fields and from them extract the extension number.
    my($a_ext) = $a->{'AOR'} =~ /sip:(\d+)\@/;
    my($b_ext) = $b->{'AOR'} =~ /sip:(\d+)\@/;
    # Compare the extensions numerically.
    return $a_ext <=> $b_ext;
}

# Extract the user-agent info from a log file.
sub findUserAgentValues {
    my @log_archives=("@SIPX_LOGDIR@/sipregistrar.log","@SIPX_LOGDIR@/sipregistrar.log.1");
    my($callid, $user_agent);

    my ($log_archive, $log);

    # In a scalar context, '%need_user_agent' is true if it is not empty.
    while (%need_user_agent &&
	   ($log_archive = shift @log_archives) ne '' &&
	   -e $log_archive)
    {
        $log=File::ReadBackwards->new($log_archive)
            || die "error opening '$log_archive'\n\t$?\n";

        # Read through the log file and find all the REGISTERs.
        my($log_line) = '';
        while ( %need_user_agent && ! $log->eof) {
            $_=$log->readline;
            next unless /:INCOMING[:]/; # the [:] makes perl-mode happier
            next unless /----\\nREGISTER\s/i;
            # This line passes the tests, process it.

            # Normalize the log line.
            s/^.*?----\\n//;
            s/====*END====*\\n"\n$//;                       # " fix syntax coloring
            s/\\(.)/$unescape{$1}/eg;
            s/\r\n/\n/g;

            # Get the Call-Id and User-Agent headers.
            ($callid) = /\nCall-Id:\s*(.*)\n/i;
            ($user_agent) = /\nUser-Agent:\s*(.*)\n/i;

            if ( $callid && $need_user_agent{$callid} ) {
                $user_agent{$callid} = $user_agent;

                # Remove this id from the set we are looking for.
                delete $need_user_agent{$callid};
            }
        }
        $log->close;
    }
}

# Turn a string into the representation we want for it in the table:
# Escape HTML special characters.
# Put <wbr/> tags between individual characters, so fields fold, if requested
# by second argument.
# Replace empty strings with &nbsp; so their borders show in IE.
# Put into code font, if requested by second argument.
sub show {
    my($string, $code) = @_;
    my($r);

    if ($string eq '') {
        $r = "&nbsp;";
    } else {
        my(@chars) = split(//, $string);
        @chars = map { $_ eq '<' ? '&lt;' :
                           $_ eq '>' ? '&gt;' :
                           $_ eq '&' ? '&amp;' :
                           $_ eq ' ' ? '&nbsp;' :
                           $_ } @chars;
        $r = $code ? &code(join('<wbr/>', @chars)) : join('', @chars);
    }
    return $r;
}

sub relativeTime
{
    my $difference = shift;
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) =
        gmtime($difference);

    return sprintf( "%5d secs (%02d:%02d:%02d)", $difference, $hour, $min, $sec);
}
