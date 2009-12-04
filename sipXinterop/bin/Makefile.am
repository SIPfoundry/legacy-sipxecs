## Process this file with automake to produce Makefile.in
include ../config/subdir.am

include ../interop.am

EXTRA_DIST = sipxinterop-setup.in

bin_SCRIPTS = sipxinterop-setup

sipxinterop-setup : $(srcdir)/sipxinterop-setup.in
	@echo "Localize $< -> $@"
	@$(INTEROP_SUBSTITUTE) $< > $@
	chmod +x $@
