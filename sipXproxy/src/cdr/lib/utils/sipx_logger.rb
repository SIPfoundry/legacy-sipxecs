#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'logger'

# Subclass the Logger to add richer logging.  The longer-term solution is likely
# to be switching to log4r.
class SipxLogger < Logger
  
  # Sipx log levels
  SIPX_DEBUG   = 'DEBUG' 
  SIPX_INFO    = 'INFO'
  SIPX_NOTICE  = 'NOTICE'
  SIPX_WARNING = 'WARNING'
  SIPX_ERR     = 'ERR'
  SIPX_CRIT    = 'CRIT'
  SIPX_ALERT   = 'ALERT'
  SIPX_EMERG   = 'EMERG'
  
  # Map the names of sipX log levels (DEBUG, INFO, NOTICE, WARNING, ERR, CRIT,
  # ALERT, EMERG) to equivalent Logger log levels.
  LOG_LEVEL_SIPX_TO_LOGGER = {
    SIPX_DEBUG   => DEBUG, 
    SIPX_INFO    => INFO, 
    SIPX_NOTICE  => INFO, 
    SIPX_WARNING => WARN,
    SIPX_ERR     => ERROR, 
    SIPX_CRIT    => FATAL,
    SIPX_ALERT   => FATAL,
    SIPX_EMERG   => FATAL
  }
  
  # Map from Logger log levels to sipX log levels
  LOG_LEVEL_LOGGER_TO_SIPX = {
    'DEBUG' => SIPX_DEBUG, 
    'INFO'  => SIPX_INFO, 
    'WARN'  => SIPX_WARNING,
    'ERROR' => SIPX_ERR, 
    'FATAL' => SIPX_CRIT
  }
  
  
  # Initializer takes any args and passes them through to Logger
  def initialize(*args)
    super
    @formatter = SipxFormatter.new    
  end
  
  
  # trying to imitate sipx log file format
  class SipxFormatter
  
    def call(severity, time, progname, msg)
      sipx_severity = LOG_LEVEL_LOGGER_TO_SIPX[severity]
      time_str = time.utc.strftime("%Y-%m-%dT%H:%M:%S.") << "%06dZ" % time.usec
      %Q<"#{time_str}":#{sipx_severity}:#{msg.chomp}\n>
    end

  end
  
end
