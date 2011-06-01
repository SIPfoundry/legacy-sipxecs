include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/ruby.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(sipXconfig_agent_SRC) \
	sipXconfig-agent.gemspec.in \
	sipxconfig-agent.wsdl

noinst_DATA = \
	sipXconfig-agent-$(sipXconfig_agent_VER).gem

sipXconfig_agent_SRC = $(shell cd $(srcdir); find lib -name '*.rb')
