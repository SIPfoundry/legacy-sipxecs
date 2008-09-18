## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

EXTRA_DIST = \
  sipregistrar.process.xml.in \
  sipXregistry-process.xml.in

# Install files in $(editetc_IN), or if they already exist, edit
# them to be compatible with the current code.

install-data-hook : \
	$(DESTDIR)$(SIPX_CONFDIR)/process.d/sipregistrar.process.xml

SIPX_PROCESS_DEFS=sipXregistry

