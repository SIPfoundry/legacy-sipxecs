include $(top_srcdir)/config/utility.am

cfgdir = $(SIPX_CONFDIR)/counterpath
dist_cfg_DATA = \
	counterpath/contacts-resource-list.xml.vm \
	counterpath/counterpath.ini.vm \
	counterpath/line.properties \
	counterpath/line.xml \
	counterpath/phone.properties \
	counterpath/phone.xml

cfg_DATA = \
	cmcprov.properties

$(cfg_DATA) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

EXTRA_DIST = $(cfg_DATA:=.in)
CLEANFILES = $(cfg_DATA)

