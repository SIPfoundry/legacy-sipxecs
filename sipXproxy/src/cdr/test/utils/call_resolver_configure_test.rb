#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'test/unit'

# set up the load path
$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))

require 'utils/call_resolver_configure'
require 'utils/configure'
require 'utils/exceptions'
require 'utils/sipx_logger'
require 'utils/utils'

class CallResolverConfigureTest < Test::Unit::TestCase

  def setup
    @config = CallResolverConfigure.default
  end

  def test_from_file
    assert_not_nil(CallResolverConfigure.from_file())
  end

  def test_get_log_dir_config
    assert_not_nil(@config.log)
    log_dir = 'No\phone\I\just\want\to\be\alone\today'

    assert_equal(STDOUT, @config.get_log_device(log_dir))


    Configure.new(CallResolverConfigure::LOG_DIR_CONFIG => log_dir)
    assert_equal(STDOUT, @config.get_log_device(log_dir))
  end

  def test_set_log_level_config

    # Pass in an empty config, should get the default log dir value
    assert_equal(Logger::INFO, @config.log.level)

    # Pass in level names, get the right values
    SipxLogger::LOG_LEVEL_SIPX_TO_LOGGER.each do |key, value|
      c = Configure.new(CallResolverConfigure::LOG_LEVEL_CONFIG => key)

      assert_equal(value, CallResolverConfigure.new(c).log.level)
    end

    # Don't allow unknown log levels
    assert_raise(CallResolverException) do
      c = Configure.new(CallResolverConfigure::LOG_LEVEL_CONFIG => 'Unknown log level name')
      CallResolverConfigure.new(c)
    end
  end

  def test_set_purge_config
    # Pass in an empty config, should get the default value of true
    assert(@config.purge?)

    # Pass in ENABLE, get true
    # Pass in DISABLE, get false
    # Comparison is case-insensitive
    c = Configure.new({CallResolverConfigure::PURGE => 'EnAbLe'})
    assert(CallResolverConfigure.new(c).purge?)

    c = Configure.new({CallResolverConfigure::PURGE => 'dIsAbLe'})
    assert(!CallResolverConfigure.new(c).purge?)

    # Pass in bogus value, get exception
    c = Configure.new({CallResolverConfigure::PURGE => 'jacket'})
    assert_raise(ConfigException) do
      assert(CallResolverConfigure.new(c).purge?)
    end
  end

  def test_get_purge_start_time_cdr_config
    assert_equal(@config.purge_age_cdr, CallResolverConfigure::PURGE_AGE_CDR_DEFAULT)

    c = Configure.new(CallResolverConfigure::PURGE_AGE_CDR => '23')
    config = CallResolverConfigure.new(c)
    assert(config.purge_age_cdr, 23)

    c = Configure.new(CallResolverConfigure::PURGE_AGE_CDR => '23', CallResolverConfigure::PURGE => 'DISABLE')
    config = CallResolverConfigure.new(c)
    assert_nil(config.purge_age_cdr)
  end

  def test_get_purge_start_time_cse_config
    assert(@config.purge_age_cse,  CallResolverConfigure::PURGE_AGE_CDR_DEFAULT)

    c = Configure.new(CallResolverConfigure::PURGE_AGE_CSE => '23')
    config = CallResolverConfigure.new(c)
    assert(config.purge_age_cse, 23)

    c = Configure.new(CallResolverConfigure::PURGE_AGE_CSE => '23', CallResolverConfigure::PURGE => 'DISABLE')
    config = CallResolverConfigure.new(c)
    assert_nil(config.purge_age_cse)
  end

  def test_cse_database_urls
    # Create URLs given the host port list
    cse_hosts = [CseHost.new("a", 1), CseHost.new("a", 2), CseHost.new("a", 3)]
    urls = @config.get_cse_database_urls_config(cse_hosts)
    assert_equal(3, urls.size)
    urls.each_with_index do |url, i|
      assert_equal('SIPXCDR', url.database)
      assert_equal(cse_hosts[i].port, url.port)
      assert_equal('localhost', url.host)
    end

    # If the host port list is empty, then we should get the default URL
    urls = @config.get_cse_database_urls_config([])
    assert_equal(1, urls.size)
    url = urls[0]
    assert_equal('SIPXCDR', url.database)
    assert_equal(5432, url.port)
    assert_equal('localhost', url.host)
  end

  def test_get_cse_hosts_config
    # Get the default entry - array with length 1 and default port
    cse_hosts, ha = @config.get_cse_hosts_config

    assert_equal(1, cse_hosts.length)
    assert_equal(5432, cse_hosts[0].port)
    assert(!ha)

    # Pass in other list, no localhost
    hostString = 'test.example.com:5433, test2.example.com:5434'
    c = Configure.new(CallResolverConfigure::CSE_HOSTS => hostString)
    config = CallResolverConfigure.new(c)

    cse_hosts, ha = config.get_cse_hosts_config

    assert_equal(2, cse_hosts.length)
    assert(ha)
    assert(5433, cse_hosts[0].port)
    assert('test.example.com', cse_hosts[0].host)
    assert(5434, cse_hosts[1].port)

    # Pass in other list, localhost, no port
    hostString = 'test.example.com:5433,localhost'
    c = Configure.new(CallResolverConfigure::CSE_HOSTS => hostString)
    config = CallResolverConfigure.new(c)

    cse_hosts, ha = config.get_cse_hosts_config

    assert_equal(2, cse_hosts.length)
    assert(ha)
    assert(5433, cse_hosts[0].port)
    assert(5432, cse_hosts[1].port)

    # Pass in other list, localhost, no port
    hostString = 'test.example.com:5433,localhost:6666'
    c = Configure.new(CallResolverConfigure::CSE_HOSTS => hostString)
    config = CallResolverConfigure.new(c)

    cse_hosts, ha = config.get_cse_hosts_config

    assert_equal(2, cse_hosts.length)
    assert(ha)
    assert(5433, cse_hosts[0].port)
    assert(6666, cse_hosts[1].port)
  end

  def test_parse_int_param
    # Check that we get the default value for an undefined param
    default = 666
    assert_equal(default, @config.send(:parse_int_param, {}, 'UNDEFINED_PARAM', default))

    # Check min and max constraints
    config_param = 'PARAM'
    config = {config_param => '10'}
    # Param exceeds the max value
    assert_raise(ConfigException) do
      @config.send(:parse_int_param, config, 'PARAM', default, 0, 5)
    end
    # Param is below the min value
    assert_raise(ConfigException) do
      @config.send(:parse_int_param, config, 'PARAM', default, 15, 20)
    end
    # Param is OK
    assert_nothing_thrown do
      @config.send(:parse_int_param, config, 'PARAM', default, 0, 20)
    end

    # Param is not an integer, should blow up
    config = {config_param => 'zax'}
    assert_raise(ConfigException) do
      @config.send(:parse_int_param, config, 'PARAM')
    end
  end

  def test_get_agent
    assert_equal('0.0.0.0', @config.agent_address)
    assert_equal(8130, @config.agent_port)
  end

  def test_cse_polling_interval
    assert_equal(10, @config.cse_polling_interval)
    c = Configure.new('SIP_CALLRESOLVER_CSE_POLLING_INTERVAL' => '12')
    config = CallResolverConfigure.new(c)
    assert_equal(12, config.cse_polling_interval)
  end

  def test_max_call_len
    assert_equal(28800, @config.max_call_len)
    c = Configure.new('SIP_CALLRESOLVER_MAX_CALL_LEN' => '-1')
    config = CallResolverConfigure.new(c)
    assert_equal(-1, config.max_call_len)
  end

  def test_max_failed_wait
    assert_equal(180, @config.max_failed_wait)
    c = Configure.new('SIP_CALLRESOLVER_MAX_FAILED_WAIT' => '60')
    config = CallResolverConfigure.new(c)
    assert_equal(60, config.max_failed_wait)
  end

  def test_min_cleanup_interval
    assert_equal(300, @config.min_cleanup_interval)
    c = Configure.new('SIP_CALLRESOLVER_MIN_CLEANUP_INTERVAL' => '500')
    config = CallResolverConfigure.new(c)
    assert_equal(500, config.min_cleanup_interval)
  end

  def test_dirs
    assert_equal('/etc/sipxpbx', @config.confdir)
    assert_equal('/etc/sipxpbx/ssl', @config.ssldir)
    assert_equal('/var/log/sipxpbx', @config.logdir)
  end

  def test_queue_sizes
    assert_equal(1000, @config.cse_queue_size)
    assert_equal(1000, @config.cdr_queue_size)
    c = Configure.new('SIP_CALLRESOLVER_CSE_QUEUE_SIZE' => '500', 'SIP_CALLRESOLVER_CDR_QUEUE_SIZE' => '550')
    config = CallResolverConfigure.new(c)
    assert_equal(500, config.cse_queue_size)
    assert_equal(550, config.cdr_queue_size)
  end
end
