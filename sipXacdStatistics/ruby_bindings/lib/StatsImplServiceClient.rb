#!/usr/bin/env ruby
require 'defaultDriver.rb'

endpoint_url = ARGV.shift
obj = AcdStatsService.new(endpoint_url)

# run ruby with -d to see SOAP wiredumps.
obj.wiredump_dev = STDERR if $DEBUG

# SYNOPSIS
#   getBirdArray
#
# ARGS
#   N/A
#
# RETURNS
#   getBirdArrayResponse ArrayOfBird - {urn:StatsService}ArrayOfBird
#

puts obj.getBirdArray

# SYNOPSIS
#   getCallStats
#
# ARGS
#   N/A
#
# RETURNS
#   getCallStatsResponse ArrayOfCallStats - {urn:StatsService}ArrayOfCallStats
#

puts obj.getCallStats

# SYNOPSIS
#   getAgentStats
#
# ARGS
#   N/A
#
# RETURNS
#   getAgentStatsResponse ArrayOfAgentStats - {urn:StatsService}ArrayOfAgentStats
#

puts obj.getAgentStats

# SYNOPSIS
#   getQueueStats
#
# ARGS
#   N/A
#
# RETURNS
#   getQueueStatsResponse ArrayOfQueueStats - {urn:StatsService}ArrayOfQueueStats
#

puts obj.getQueueStats

# SYNOPSIS
#   getCallHistory(fromTime)
#
# ARGS
#   fromTime        DateTime - {http://www.w3.org/2001/XMLSchema}dateTime
#
# RETURNS
#   callHistory     ArrayOfCallAudit - {urn:StatsService}ArrayOfCallAudit
#
fromTime = nil
puts obj.getCallHistory(fromTime)

# SYNOPSIS
#   getAgentHistory(fromTime)
#
# ARGS
#   fromTime        DateTime - {http://www.w3.org/2001/XMLSchema}dateTime
#
# RETURNS
#   agentHistory    ArrayOfAgentAudit - {urn:StatsService}ArrayOfAgentAudit
#
fromTime = nil
puts obj.getAgentHistory(fromTime)


