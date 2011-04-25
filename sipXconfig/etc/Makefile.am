include $(top_srcdir)/config/subdir.am

EXTRA_DIST = \
	$(procdef_DATA:=.in)

procdefdir = $(SIPX_DATADIR)/process.d
procdef_DATA = tunnel-process.xml

$(procdef_DATA) : % : %.in Makefile
	$(call SearchAndReplace, $<, $@)
