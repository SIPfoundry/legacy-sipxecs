include $(top_srcdir)/config/subdir.am

PREVIOUS_CONFIG = $(DESTDIR)@SIPX_UPGRADEDIR@/@PACKAGE@-previous.tgz

install-data-hook : $(PREVIOUS_CONFIG)

$(PREVIOUS_CONFIG) :
	$(INSTALL) -D -m 644 $(PREVIOUS_CONFIG)



