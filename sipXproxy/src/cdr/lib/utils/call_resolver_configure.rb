#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'time'

require 'db/database_url'
require 'utils/configure'
require 'utils/exceptions'
require 'utils/sipx_logger'


class CseHost < Struct.new(:host, :port, :local)
end

class CallResolverConfigure

  # Default config file path
  DEFAULT_CONF_DIR = '/etc/sipxpbx'
  DEFAULT_LOG_DIR = '/var/log/sipxpbx'

  CONFIG_FILE_NAME = 'callresolver-config'
  LOG_FILE_NAME = 'sipxcallresolver.log'

  LOCALHOST = 'localhost'

  # Max integer in a Fixnum, on a 32-bit machine
  INT_MAX = 2147483647

  # Configuration parameters and defaults

  # Whether console logging is enabled or disabled.  Legal values are "ENABLE"
  # or "DISABLE".  Comparison is case-insensitive with this and other values.
  LOG_CONSOLE_CONFIG = 'SIP_CALLRESOLVER_LOG_CONSOLE'

  # The directory holding log files.
  LOG_DIR_CONFIG = 'SIP_CALLRESOLVER_LOG_DIR'
  LOG_DIR_CONFIG_DEFAULT = '/var/log/sipxpbx'

  # Logging severity level
  LOG_LEVEL_CONFIG = 'SIP_CALLRESOLVER_LOG_LEVEL'
  LOG_LEVEL_CONFIG_DEFAULT = 'NOTICE'

  PURGE = 'SIP_CALLRESOLVER_PURGE'
  PURGE_DEFAULT = Configure::ENABLE

  PURGE_AGE_CDR = 'SIP_CALLRESOLVER_PURGE_AGE_CDR'
  PURGE_AGE_CDR_DEFAULT = 35

  PURGE_AGE_CSE = 'SIP_CALLRESOLVER_PURGE_AGE_CSE'
  PURGE_AGE_CSE_DEFAULT = 7

  CSE_HOSTS = 'SIP_CALLRESOLVER_CSE_HOSTS'

  class << self
    def from_file(confdir = DEFAULT_CONF_DIR, logdir = DEFAULT_LOG_DIR)
      config_file =  File.join(confdir, CONFIG_FILE_NAME)
      configure = if File.exists?(config_file)
        Configure.from_file(config_file)
      else
        $stderr.puts("Config file #{config_file} not found, using default settings")
        Configure.new()
      end

      CallResolverConfigure.new(configure, confdir, logdir)
    end

    def default
      configure = Configure.new()
      CallResolverConfigure.new(configure)
    end
  end

  attr_reader :cdr_database_url, :cse_database_urls, :cse_hosts, :log, :confdir, :logdir, :db_user

  def initialize(config, confdir = DEFAULT_CONF_DIR, logdir = DEFAULT_LOG_DIR)
    @config = config
    @confdir = confdir
    @logdir = logdir

    # Read logging config and initialize logging.  Do this before initializing
    # the rest of the config so we can use logging there.
    @log = init_logging(logdir)

    # Finish setting up the config.  Logging has already been set up before this,
    # so we can log messages in the methods that are called here.

    @db_user = config.fetch('SIP_CALLRESOLVER_DB_USER', 'postgres')

    # default values for db connection info
    @local_db_url = DatabaseUrl.new(:username => @db_user)

    # :TODO: read CDR database URL params from the Call Resolver config file
    # rather than just hardwiring default values.
    @cdr_database_url = DatabaseUrl.new(:username => @db_user)

    # These two methods must get called in this order
    @cse_hosts, @ha = get_cse_hosts_config
    @cse_database_urls = get_cse_database_urls_config(@cse_hosts)
  end

  # Return true if High Availability (HA) is enabled, false otherwise
  def ha?
    @ha
  end

  # Return true if database purging is enabled, false otherwise
  def purge?
    @config.enabled?(PURGE, PURGE_DEFAULT)
  end

  # Compute start time of CDR records to be purged from configuration
  def purge_age_cdr
    return nil unless purge?
    return parse_int_param(@config, PURGE_AGE_CDR, PURGE_AGE_CDR_DEFAULT, 1)
  end

  # Compute start time of CSE records to be purged from configuration
  def purge_age_cse
    return nil unless purge?
    return parse_int_param(@config, PURGE_AGE_CSE, PURGE_AGE_CSE_DEFAULT, 1)
  end

  def agent_port
    @config.fetch('SIP_CALLRESOLVER_AGENT_PORT', 8130)
  end

  def agent_address
    @config.fetch('SIP_CALLRESOLVER_AGENT_ADDR', '0.0.0.0')
  end

  # number of seconds between attempts to read new CSEs from database
  def cse_polling_interval
    parse_int_param(@config, 'SIP_CALLRESOLVER_CSE_POLLING_INTERVAL', 10, 1)
  end

  # number of seconds for that the longest registered call may last
  # all calls longer than max_call_len will be logged as completed
  # -1 means - no lenght limit (but it may lead to zombie calls never removed
  # from the pool of active calls)
  def max_call_len
    parse_int_param(@config, 'SIP_CALLRESOLVER_MAX_CALL_LEN', 8 * 60 * 60, -1)
  end

  # Maximum time an unanswered ringing call is allowed before being marked as failed.
  def max_ringing_call_len
    parse_int_param(@config, 'SIP_CALLRESOLVER_MAX_RINGING_CALL_LEN', 2 * 60, -1)
  end

  # number of seconds after which filed calls will be recorded if no new leg open
  def max_failed_wait
    parse_int_param(@config, 'SIP_CALLRESOLVER_MAX_FAILED_WAIT', 3 * 60, -1)
  end

  # number of seconds to wait between cleaning up long calls.  Default is
  # 5 minutes.  Minimum is 1 minute
  def min_cleanup_interval
    parse_int_param(@config, 'SIP_CALLRESOLVER_MIN_CLEANUP_INTERVAL', 5 * 60 ,60)
  end

  def stunnel_debug
    @config.fetch('SIP_CALLRESOLVER_STUNNEL_DEBUG', 5)
  end

  def cse_connect_port
    @config.fetch('SIP_CALLRESOLVER_STUNNEL_PORT', 9300)
  end

  def cse_queue_size
    parse_int_param(@config, 'SIP_CALLRESOLVER_CSE_QUEUE_SIZE', 1000)
  end

  def cdr_queue_size
    parse_int_param(@config, 'SIP_CALLRESOLVER_CDR_QUEUE_SIZE', 1000)
  end

  def ssldir
    File.join(confdir, 'ssl')
  end

  # Access the config as an array.  Use this method *only* for plugin config
  # params that are unknown to the call resolver.  All known params should be
  # retrieved using the above accessors.
  def [](param)
    @config[param]
  end

  def enabled?(param, default_value = nil)
    @config.enabled?(param, default_value)
  end

  # Determines the log device
  # if console logging is enabled than log to console
  # if log directory is provided in configuration log there
  # otherwise log to logdir directory passed as parameter
  def get_log_device(logdir)
    return STDOUT if @config.enabled?(LOG_CONSOLE_CONFIG, Configure::DISABLE)

    log_dir = @config.fetch(LOG_DIR_CONFIG, logdir)
    unless File.exists?(log_dir)
      $stderr.puts("init_logging: Log directory '#{@log_dir}' does not exist. " +
      "Log messages will go to the console.") if $DEBUG
      return STDOUT
    end
    log_device = File.join(log_dir, LOG_FILE_NAME)
    # If the file exists, then it must be writable. If it doesn't exist, then the directory must be writable.
    if File.exists?(log_device)
      if !File.writable?(log_device)
        $stderr.puts("init_logging: Log file '#{log_file}' exists but is not writable. " +
          "Log messages will go to the console.") if $DEBUG
        return STDOUT
      end
    else
      if !File.writable?(log_dir)
        $stderr.puts("init_logging: Log directory '#{@log_dir}' is not writable. " +
          "Log messages will go to the console.") if $DEBUG
        return STDOUT
      else
        # File doesn't exist and the directory is writable.  Create the file ourselves so that
        # we can get around Logger creating it and adding a comment at the top of the file.
        logdev = open(log_device, (File::WRONLY | File::APPEND | File::CREAT))
        logdev.close
      end
    end
    return log_device
  end

  # Set up logging.  Return the Logger.
  def init_logging(logdir)
    log = SipxLogger.new(get_log_device(logdir))

    # Set the log level from the configuration
    log.level = get_log_level_config(@config)

    # Override the log level to DEBUG if $DEBUG is set.
    # :TODO: figure out why this isn't working.
    if $DEBUG then
      log.level = Logger::DEBUG
    end

    return log
  end

  # Set the log level from the configuration.  Return the log level.
  def get_log_level_config(config)
    # Look up the config param
    log_level_name = config[LOG_LEVEL_CONFIG] || LOG_LEVEL_CONFIG_DEFAULT

    # Convert the log level name to a Logger log level
    SipxLogger::LOG_LEVEL_SIPX_TO_LOGGER.fetch(log_level_name) { | name |
      # If we don't recognize the name, then refuse to run.  Would be nice to
      # log a warning and continue, but there is no log yet!
      raise CallResolverException, "Unknown log level: #{name}"
    }
  end

  # Return an array of CSE database URLs.  With an HA configuration, there are
  # multiple CSE databases.  Note that usually one of these URLs is identical
  # to the CDR database URL, since a standard master server runs both the
  # proxies and the call resolver, which share the SIPXCDR database.
  def get_cse_database_urls_config(cse_hosts)
    return [ @cdr_database_url ] if cse_hosts.empty?
    # Build the list of CSE DB URLs.  From Call Resolver's point of view,
    # each URL is 'localhost:<port>'.
    # Stunnel takes care of forwarding the local port to the database on a remote host.
    cse_hosts.collect do |cse_host|
      DatabaseUrl.new(:port => cse_host.port, :username => @db_user)
    end
  end

  # Get distributed CSE hosts from the configuration.
  #
  # Return list of CSE host information
  #
  # Call resolver connects to each of these ports on 'localhost' via the
  # magic of stunnel, so it doesn't ever use the hostnames.
  def get_cse_hosts_config
    ha = false
    cse_hosts = []
    cse_hosts_config = @config[CSE_HOSTS] || "#{@local_db_url.host}:#{@local_db_url.port}"
    cse_hosts_config.split(',').each do |host_string|
      host, port = host_string.split(':')
      host.strip!
      local = host == LOCALHOST
      ha = true unless local
      if port
        port.strip!
      else
        if local
          # Supply default port for localhost
          port = @local_db_url.port
        else
          raise ConfigException, "No port specified for host '#{host}'. " +
            "A port number for hosts other than  'localhost' must be specified."
        end
      end
      host_port = port.to_i
      raise ConfigException, "Port for #{host} is invalid." if host_port == 0
      cse_hosts << CseHost.new(host, host_port, host == LOCALHOST)
      log.debug("cse_hosts: host name #{host}, host port: #{port}")
      # If at least one of the hosts != 'localhost' we are HA enabled
    end
    log.debug("Found host other than localhost - enable HA") if ha
    return cse_hosts, ha
  end

  # Read the named param from the config.  Convert it to an integer and return
  # the value.  If the param is not defined, then use the default.  Validate
  # that the param is between the min and max and raise a ConfigException if not.
  def parse_int_param(config, param_name, default_value = nil, min = 0, max = INT_MAX)
    param_value = default_value
    begin
      value = config[param_name]
      param_value = Integer(value) if value
      if !(min..max).include?(param_value)
        raise ConfigException, "Configuration parameter #{param_name}, #{param_value} must be in #{min}..#{max} range."
      end
    rescue ArgumentError
      raise ConfigException, "Configuration parameter #{param_name}, #{param_value} must be an integer"
    end

    param_value
  end
end
