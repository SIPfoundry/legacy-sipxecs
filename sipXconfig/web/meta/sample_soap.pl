#!/usr/bin/perl

# This script is a sample script using perl to add 10 users to a system 
# It relies on SOAP::Lite package to be installed.

use SOAP::Lite;

# helpful when debugging SOAP messages on the wire
# use SOAP::Lite +trace;
use Digest::MD5 qw(md5_hex); 
use strict;
use warnings;

# Where your server is installed, 
# CHANGE ME: probably not localhost!
my $services_url = 'https://localhost:8443/sipxconfig/services';

# CHANGE ME: Namespace may need to be changed as the SOAP service namesapce changes
my $namespace = 'http://www.sipfoundry.org/2007/08/21/ConfigService';

sub SOAP::Transport::HTTP::Client::get_basic_credentials { 
   return 'superadmin' => '1234';
}

my $system_service = SOAP::Lite
  -> uri("$namespace")
  -> proxy("$services_url/SystemService");

my $user_service = SOAP::Lite
  -> uri("$namespace")
  -> proxy("$services_url/UserService");

# find out what the realm is via a SOAP query
my $realm = find_realm();
print "obtained realm via the SOAP service: $realm\n";

foreach my $i (1 .. 10) {
  my $username = "roberta-$i";
  my $group = "employee";

  # delete to ensure this is a new user, you can always find and edit too
  delete_user(username => $username);

  add_user(username => $username, pin => '4567', sipPassword => 'DFad56sd!', groups => $group, 
	   aliases => '345' + $i);

  change_pin(username => $username, pin => '1234');
 
  add_group(username => $username, group => "building-1");
}


sub find_realm {
    my $systemInfo = SOAP::Data->name('SystemInfo')
      ->attr({xmlns => "$namespace"});

    my $result = $system_service->call($systemInfo);
    check_fault($result);

    my $domain = $result->result();
    foreach my $key (keys %{$domain}) {
      print "SystemInfo ", $key, ": ", $domain->{$key} || '', "\n";
    }
    return $domain->{'realm'};
}

sub change_pin {
    my %params = @_;
    my ($username) = $params{username};
    my ($pin) = $params{pin};

    my $pintoken = &md5_hex("$username:$realm:$pin");

    my $change_pintoken = SOAP::Data->name(edit =>
       \SOAP::Data->value(
	SOAP::Data->type('string')->name(property => 'pintoken'),
	SOAP::Data->type('string')->name(value => $pintoken)));

    print "changing pintoken for $username to $pintoken\n";
    manage_user(username => $username, action => $change_pintoken);    
}

sub find_user {
    my %params = @_;
    my ($username) = $params{username};

    my $findUser = SOAP::Data->name('FindUser')
      ->attr({xmlns => "$namespace"});

    my $byUserName = SOAP::Data->type('string')->name('byUserName')->value($username);
    my $result = $user_service->call($findUser => SOAP::Data->name(search => \$byUserName));
    check_fault($result);

    return $result;
}

sub add_user {
    my %params = @_;
    my ($username) = $params{username};
    my ($pin) = $params{pin};
    my ($sipPassword) = $params{sipPassword};
    my ($groups) = $params{groups};
    my ($aliases) = $params{aliases};

    my $newUser = SOAP::Data->name(user =>
       \SOAP::Data->value(
	SOAP::Data->type('string')->name(userName => $username),
	SOAP::Data->type('string')->name(sipPassword => $sipPassword),
        SOAP::Data->type('string')->name(groups => $groups),
        SOAP::Data->type('string')->name(aliases => $aliases)));

    my $pinData = SOAP::Data->type('string')->name(pin => $pin);

    my $addUser = SOAP::Data->name('AddUser')
       ->attr({xmlns => "$namespace"});

    print "adding user $username\n";
    my $result = $user_service->call($addUser => SOAP::Data->value($newUser, $pinData));
    check_fault($result);
}

sub check_fault {
    my $result = shift;
    if ($result->fault) {
      # some error handling
      print join ', ',
      $result->faultcode,
      $result->faultstring,
      $result->faultdetail;
  }
}

sub delete_user {
    my %params = @_;
    my ($username) = $params{username};

    my $delete_user = SOAP::Data->name('deleteUser')->type('boolean')->value("true");

    print "deleting $username\n";
    manage_user(username => $username, action => $delete_user);    
}

sub add_group {
    my %params = @_;
    my ($username) = $params{username};
    my ($group) = $params{group};

    my $add_group = SOAP::Data->name('addGroup')->value($group);

    print "adding $username to group $group\n";
    manage_user(username => $username, action => $add_group);    
}

# helper function for methods that manage a user liek edit and delete
sub manage_user {
    my %params = @_;
    my ($username) = $params{username};
    my ($action) = $params{action};

    my $byUserName = SOAP::Data->type('string')->name('byUserName')->value($username);
    my $search = SOAP::Data->name(search => \$byUserName);

    my $manageUser = SOAP::Data->name('ManageUser')
      ->attr({xmlns => "$namespace"});
    my $result = $user_service->call($manageUser => SOAP::Data->value($search, $action)); 
    check_fault($result);   
}

