## Process this file with automake to produce Makefile.in

include $(top_srcdir)/config/subdir.am

unconditional_SUBDIRS = . src bin meta etc 
if DOC
doc_SUBDIRS=doc
endif

editscripts_IN = \
    bin/sipxrls.sh.in

editscripts :
	@for f in $(editscripts_IN); \
	do \
	  target=`basename $$f .in`; \
          $(LocalizeSipXconfig) $(srcdir)/$$f > $$target; \
	  $(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)
RELEASE = 1
