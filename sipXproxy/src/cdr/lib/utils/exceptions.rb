#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

# CallResolverException is the base class for all Call Resolver exceptions.
class CallResolverException < StandardError
end

class BadContactException < CallResolverException
  attr_reader :contact
  
  def initialize(contact)
    super
    @contact = contact
  end
end

class BadSipHeaderException < CallResolverException
  attr :header
  
  # Construct BadSipHeaderException with the text from the bad header.
  def initialize(header)
    super
    @header = header
  end
  
  def to_s
    super + ": \"#{header}\""
  end
  
end

class ConfigException < CallResolverException
end

class NameResolutionException < CallResolverException
  attr_accessor :domain_name    # the domain name that failed to resolve
  
  def initialize(domain_name)
    super
    @domain_name = domain_name
  end
end
