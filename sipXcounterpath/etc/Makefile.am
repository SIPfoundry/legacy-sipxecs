include $(top_srcdir)/config/utility.am

cfinputsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfinputs_DATA = \
	sipxcounterpath.cf

cfgdir = $(SIPX_CONFDIR)/counterpath
dist_cfg_DATA = \
	counterpath/contacts-resource-list.xml.vm \
	counterpath/counterpath.ini.vm \
	counterpath/line.properties \
	counterpath/line.xml \
	counterpath/phone.properties \
	counterpath/phone.xml

cfg_DATA = \
	cmcprov.properties \
	httpd-dav.conf	

$(cfg_DATA) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)
	
emptydirs = \	
	$(DESTDIR)$(wwwdir)/var \	
	$(DESTDIR)$(wwwdir)/webdav	

EXTRA_DIST = \ 
	$(cfg_DATA:=.in) \
	$(webdav_DATA:=.in)

CLEANFILES = \ 
	$(cfg_DATA)

$(emptydirs) :
	$(INSTALL) -d $@

install-data-hook : $(emptydirs)
