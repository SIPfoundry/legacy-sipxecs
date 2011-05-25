include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/ruby.am

sipxacd_stats_client_VER = 1.0.0

noinst_DATA = \
	sipxacd-stats-client-$(sipxacd_stats_client_VER).gem

WSDL = $(abspath $(srcdir)/../agent/sipxconfig-agent.wsdl)

sipxacd_stats_client_SRC = \
	lib/defaultDriver.rb \
	lib/default.rb \
	lib/StatsImplServiceClient.rb

sipxacd-stats-client-clean : rm-src

rm-src :
	-rm $(sipxacd_stats_client_SRC)

$(sipxacd_stats_client_SRC) : $(WSDL)
	@echo "$$wsdl2ruby" | Wsdl=$< ruby
