## Process this file with automake to produce Makefile.in

include $(top_srcdir)/config/subdir.am

unconditional_SUBDIRS = . src bin etc

include config/conditional_doc.am

editscripts_IN = \
    bin/sipxpark.sh.in

editscripts :
	@for f in $(editscripts_IN); \
	do \
	  target=`basename $$f .in`; \
     $(LocalizeSipXconfig) $(srcdir)/$$f > $$target; \
	  $(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)
RELEASE = 1

