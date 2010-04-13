#!/usr/bin/env ruby
#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

# system requires
require 'getoptlong'
require 'parsedate'

# FIXME: that should not be required if gems are used properly
$:.unshift(File.dirname(__FILE__))

# application requires
require 'call_resolver'
require 'utils/call_resolver_configure'
require 'utils/stunnel_connection'


def usage()
  print <<EOT
usage: sipxcallresolver.sh (--start "time" [--end "time"]|--daemon) [--help]

  --start "time"  Specifies the start time of the window in which calls are to
                  be resolved.
  --end "time"    Specifies the end time of the window in which call are to be
                  resolved.
  --daemon        Makes sipXcondig resolver to stay active after processing initial
                  batch of CSE records. New records are processed as they appear.
EOT
  exit 1
end

# ruby-postgress RPM installs postgres.so in /usr/lib/site_ruby/1.8/i386-linux
# however on CentOS ruby 'sitearchdir' is /usr/lib/site_ruby/1.8/i386-linux-gnu
# this function tries to load postgres from both places
# On Suse 'postgres' is installed as ruby gem: loading rubygems explictely enables
# us to find it.
def load_postgres_driver()
  require 'postgres'
rescue LoadError
  require 'rubygems'
  require 'rbconfig'
  sad = Config::CONFIG['sitearchdir']
  postgres_dir = sad.chomp('-gnu')
  $:.unshift(postgres_dir)
  require 'postgres'
rescue LoadError
  # On SUSE, FreeBSD the driver is called ruby-postgres and is a gem
  require 'ruby-postgres'
end

def main()
  opts = GetoptLong.new(
                        [ "--start", "-s", GetoptLong::REQUIRED_ARGUMENT ],
  [ "--end",   "-e", GetoptLong::REQUIRED_ARGUMENT ],
  [ "--daemon", GetoptLong::NO_ARGUMENT ],
  [ "--help",  "-h", GetoptLong::NO_ARGUMENT ],
  [ "--confdir", GetoptLong::REQUIRED_ARGUMENT ],
  [ "--logdir", GetoptLong::REQUIRED_ARGUMENT ]
  )

  # Init options
  start_time = nil
  end_time = Time.now
  daemon_flag = false
  confdir =  CallResolverConfigure::DEFAULT_CONF_DIR
  logdir = CallResolverConfigure::DEFAULT_LOG_DIR

  # Extract option values
  # Convert start and end strings to date/time values.
  opts.each do |opt, arg|
    case opt

      when "--start"
      start_time = Time.parse(arg)

      when "--end"
      end_time = Time.parse(arg)

      when "--daemon"
      daemon_flag = true

      when "--confdir"
      confdir = arg

      when "--logdir"
      logdir = arg

    else
      usage
    end
  end

  usage unless daemon_flag || (start_time && end_time)

  config = CallResolverConfigure.from_file(confdir, logdir)
  load_postgres_driver()

  resolver = CallResolver.new(config)

  stunnel_connection = StunnelConnection.new(config)

  stunnel_connection.open()

  resolver.check_connections
  if daemon_flag
    resolver.run_resolver
  else
    resolver.resolve(start_time, end_time)
  end

rescue
  config.log.error("Exiting because of error: <#{$!}>")
  config.log.error do
    start_line = "\n"
    $!.backtrace.inject("") {|trace, line| "#{trace}\n#{line}" }
  end
ensure
  stunnel_connection.close if stunnel_connection
end


if __FILE__ == $0
  main()
end
