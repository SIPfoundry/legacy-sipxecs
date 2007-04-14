#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

# Holds the params needed to connect to a database
class DatabaseUrl < Struct.new(:database, :port, :host, :adapter, :username)
  
  DATABASE_DEFAULT = 'SIPXCDR'
  DATABASE_PORT_DEFAULT = 5432                # Default port used by PostgreSQL
  HOST_DEFAULT     = 'localhost'  
  ADAPTER_DEFAULT  = 'postgresql'
  USERNAME_DEFAULT = 'postgres'
  
  def initialize(args = nil)
    super()
    args.each do |field, value| 
        self[field] = value
    end if args
    self[:database] ||= DATABASE_DEFAULT
    self[:port] ||= DATABASE_PORT_DEFAULT
    self[:host] ||= HOST_DEFAULT
    self[:adapter] ||= ADAPTER_DEFAULT
    self[:username] ||= USERNAME_DEFAULT
  end
  
  def to_dbi
    "dbi:Pg:database=#{database};host=#{host};port=#{port}"
  end
end
