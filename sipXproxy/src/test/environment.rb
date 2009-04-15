#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

# system requirements
require 'getoptlong'
require 'digest/md5'

$testUsers = [ 'superadmin', '200', '201', '202', '203' ] 

$testSipURIs= [] 

# Get options from the command line

def getOptions
  opts = GetoptLong.new(
    [ "--prefix",  "-p",   GetoptLong::REQUIRED_ARGUMENT ],
    [ "--dbname",  "-d",   GetoptLong::REQUIRED_ARGUMENT ],
    [ "--verbose", "-v",   GetoptLong::NO_ARGUMENT       ]
  )
  $verbose = false
  opts.each do |opt, arg|
    case opt
      when "--prefix" then $prefix = arg
      when "--dbname" then $dbname = arg.inspect
      when "--verbose" then $verbose = true
    end
  end
end


# Set up some variables describing the enviroment the tests
# are run in.

def setEnvironment
  getOptions

  if $verbose
    puts "Prefix: #{$prefix}"
    puts "DbName: #{$dbname}"
  end
   
  shellResult = `hostname -i`
  $ipAddress = shellResult.chomp

  shellResult = `hostname -f`
  $hostName = shellResult.chomp

  $realm = $hostName[$hostName.index('.')+1, $hostName.length]

  $proxyPort = 5060
  $registrarPort = 5070
  $authproxyPort = 5080

  # Build a list of SIP URIs  
  $testUsers.each do |user|
     $testSipURIs.concat([ "sip:#{user}@#{$ipAddress}" ])
  end 

  puts $testSipURIs

  $firstParty = "sip:500@" + $hostName
  $secondParty = "sip:501@" + $hostName
  $thirdParty = "sip:502@" + $hostName

  $firstPartyIp = "sip:500@" + $ipAddress
  $secondPartyIp = "sip:501@" + $ipAddress
  $thirdPartyIp = "sip:502@" + $ipAddress

  $userAgent = "CseUnitTest/0.01"
end


# Creates a ${prefix}/var/sipxdata/sipdb/credential.xml file with
# entries for all users defined in the testUsers array. May
# not be necessary if the registrar is not running.

def createCredentials
  puts "Generating credentials..." if $verbose

  hash_class = Digest::MD5

  credentialFileName = $prefix + \
                "/var/sipxdata/sipdb/credential.xml"

  fileObj = File.new(credentialFileName, \
                     File::CREAT|File::TRUNC|File::RDWR, 0644)
  fileObj.syswrite("<?xml version=\"1.0\" standalone=\"yes\" ?>\n")
  fileObj.syswrite("<items type=\"credential\">\n")
  $testUsers.each do |user|
    fileObj.syswrite("  <item>\n")
    fileObj.syswrite("    <realm>#{$realm}</realm>\n")
    fileObj.syswrite("    <uri>sip:#{user}@#{$hostName}</uri>\n")
    fileObj.syswrite("    <userid>#{user}</userid>\n")
    hashstring = hash_class.hexdigest("#{user}:#{$realm}:1234")
    fileObj.syswrite("    <passtoken>#{hashstring}</passtoken>\n")
    fileObj.syswrite("    <pintoken>#{hashstring}</pintoken>\n")
    fileObj.syswrite("  </item>\n")
  end
  fileObj.syswrite("</items>\n")
  fileObj.close
end

# Start proxy, clear any old logs

def startServices
  logs = $prefix + "/var/log/sipxpbx/*";
  proxyStartup = $prefix + "/bin/sipXproxy &"
  
  puts "Removing logs..." if $verbose

  system("sudo rm -rf #{logs}")

  puts "Starting proxy service..." if $verbose

  fork do exec(proxyStartup) end

  # Give it time to start up
  sleep 1

  # Do a funky way of retrieving PID for sipXproxy,
  # later stop them by killing them. This is necessary because
  # there seems to be no way to programmatically retrieve
  # a process id of an external process. A 'spawn' function
  # that returns the PID of an external process is promised
  # for Ruby 1.9.

  shellReturn = `ps -C sipXproxy | cut -d ' ' -f 1`
  proxyPidString = shellReturn.strip

  $proxyPid = proxyPidString.to_i

  puts "Running processes #{$proxyPid}" \
    if $verbose
  $proxyPid != 0
end

def stopServices
  puts "Stopping services (#{$proxyPid})" \
    if $verbose

  Process.kill(3, $proxyPid)
  
  # Give them time to shut down
  sleep 1
end

def checkForZombieProxies
  puts "Checking for zombie proxies" if $verbose

  proxyPidString = ''

  shellReturn = `ps -C sipXproxy | cut -d ' ' -f 1`
  proxyPidString = shellReturn.strip

  proxyPid = proxyPidString.to_i

  puts "Leftover process #{proxyPid}" \
    if $verbose

  proxyPid == 0
end
