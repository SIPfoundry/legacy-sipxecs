#!/usr/bin/env ruby
#
# Copyright (C) 2008 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

# Usage: polycom_config_dump.rb [URL]
# Reads the specified Polycom SoundPoint/SoundStation IP configuration files and extracts
# the individual sip and phone1 configuration parameters, which it dumps (sorted) into a flat
# INI file.  (DUMP_<MAC>.ini)  It handles multiple configuration files and applies overrides
# in the same manner as the actual phone.

# Default URL value.
root_uri_str = "file:///./000000000000.cfg"
# Other Example: root_uri_str = "ftp://PlcmSpIp:PlcmSpIp@bcmsl2165.ca.nortel.com/0004f2064015.cfg"
# Other Example: root_uri_str = "file:///D:/Profiles/MOSSMANP/Desktop/MISC/spip_ssip_3_1_1_release_sig/000000000000.cfg"

require 'rexml/document'
require 'net/http'
require 'open-uri'
require 'uri'

include REXML

MAJOR_DELIMITOR="-"

META_PREFIX="meta#{MAJOR_DELIMITOR}"
META_COUNT_PREFIX="#{META_PREFIX}count."
META_NumberOfXmlAttributes = "XmlAttributes"
META_NumberOfOverrideAttempts = "OverrideAttempts"
META_NumberOfOverrideSuccess = "OverrideSuccess"
META_NumberOfUniqueParameters = "UniqueParameters"

if 1 <= ARGV.length 
   root_uri_str = ARGV[0]   
end

match_name="APPLICATION"
if 2 <= ARGV.length
   match_name = ARGV[1]
end

def get_file_io(uri_str)
   uri=URI::parse(uri_str)
   if "file" == uri.scheme # e.g. file:///./3.1.0/000000000000.cfg
      File.open(uri.path[1,uri.path.length], "r")
   else
      open(uri)
   end
end

def get_file_xml(uri_str)
   Document.new(get_file_io(uri_str))
end

def meta_count_key(meta_name)
   "#{META_COUNT_PREFIX}#{meta_name}"
end

def increment_count_meta(config_hash, meta_name)
   key = "#{meta_count_key(meta_name)}"
   if nil == config_hash[key] 
      config_hash[key] = 1
   else
      config_hash[key] = config_hash[key] + 1
   end
end

def initialize_count_metas(config_hash)
   config_hash[meta_count_key("#{META_NumberOfXmlAttributes}")] = 0
   config_hash[meta_count_key("#{META_NumberOfOverrideAttempts}")] = 0
   config_hash[meta_count_key("#{META_NumberOfOverrideSuccess}")] = 0
   config_hash[meta_count_key("#{META_NumberOfUniqueParameters}")] = 0
end

def add_config_from_attributes(config_hash, prefix, attributes)
   attributes.each_attribute do |attribute|
      increment_count_meta(config_hash, "#{META_NumberOfXmlAttributes}")
      key = "#{prefix}#{MAJOR_DELIMITOR}#{attribute.name}"
      if nil != config_hash[key]
         # We've already recorded a value for this parameter, so this is an override attempt.
         increment_count_meta(config_hash, "#{META_NumberOfOverrideAttempts}")
         if config_hash[key] != attribute.value
            # The new value is different, so this is a successful override.
            increment_count_meta(config_hash, "#{META_NumberOfOverrideSuccess}")
         end
      else
         # It wasn't already in the hash, so this is unique parameter.
         increment_count_meta(config_hash, META_NumberOfUniqueParameters)
      end

      # No matter what was determined above, we know this is the correct value to use.   
      config_hash[key] = attribute.value
   end
end

def add_config_from_elements(config_hash, prefix, elements)
   elements.each do |element|
      if nil != element.text  && 0 != element.text.to_s.strip.length
         puts "ERROR!  Element #{element.name} has text!! (--#{element.text.to_s.strip}--)" # This is very unexpected...
      end
      add_config_from_attributes(config_hash, prefix, element.attributes)
      add_config_from_elements(config_hash, prefix, element.elements)
   end
end

def add_config(config_hash, config_doc)
   add_config_from_elements(config_hash, config_doc.elements[1].name, config_doc.elements[1].elements)
end


def get_matching_application_element(element, match_name)
   if nil == element
      nil
   elsif match_name == element.name
      element
   else
      return_value=nil
      element.elements.each do |child|
         result=get_matching_application_element(child, match_name)
         if result != nil
            return_value=child
            break
         end
      end
      return_value
   end
end

def config_dump(root_uri_str, match_name)
   # Get the root configuration document.
   mac_cfg_doc = get_file_xml(root_uri_str)
   mac_str = File.basename(root_uri_str, ".cfg")
   base_uri_str = root_uri_str.chomp(File.basename(root_uri_str))

   # Extract the names of the configuration files.
   application_element=get_matching_application_element(mac_cfg_doc.elements[1], match_name)
   if nil == application_element
      puts "ERROR: Failed to find an application element named '#{match_name}'."
      exit 1
   end
   config_attribute_value=nil
   application_element.attributes.each do |name,value|
      if 0 == name.index("CONFIG_FILES")
         config_attribute_value=value
         break
      end
   end
   if nil == config_attribute_value
      puts "ERROR: Failed to find CONFIG_FILES attribute in the #{match_name} element."
      exit 2
   end
   config_file_basenames = config_attribute_value.split(%r{,\s*})
   # Limitation: Polycom allows for these to be full URIs.  This script assumes 
   # they are simple filenames only, and that each file will be found in the same
   # directory as the <MAC>.cfg file.

   # Polycom doc: Since the files are processed left to right, any parameter 
   # which appears in first file will override the same parameter in later files.  
   # e.g. CONFIG_FILES="0004f2000607-user.cfg, local-settings.cfg, phone1.cfg, sip.cfg"
   # To accomplish the same, this script will instead read the files in reverse
   # order and instead allow later parameter values to override earlier ones.
   config_file_basenames.reverse!

   # Initialize the hash.
   config_hash = {}
   initialize_count_metas(config_hash)
   config_hash["#{META_PREFIX}mac"] = mac_str

   # Gather the key value pairs.
   config_file_basenames.each do |config_file_basename|
      config_file_basename.gsub!(/\[PHONE_MAC_ADDRESS\]/, mac_str)
      config_doc = get_file_xml("#{base_uri_str}#{config_file_basename}")
      add_config(config_hash, config_doc)
   end 

   # Sort by key.
   sorted_nested_array = config_hash.sort

   # Dump the values to a file, but only the meta values to stdout.
   dump_ini = File.open("DUMP_#{mac_str}.ini", "w")
   sorted_nested_array.each do |pair|
      if 0 == pair[0].index(META_PREFIX)
         puts "#{pair[0]} = #{pair[1]}"
      end
      dump_ini.puts "#{pair[0]} = \"#{pair[1]}\""
   end
   dump_ini.close

end

config_dump(root_uri_str, match_name)


