include $(top_srcdir)/config/utility.am

INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = syslog2siptrace

syslog2siptrace_LDADD = \
	@SIPXTACK_LIBS@

syslog2siptrace_SOURCES = main.cpp

bin_SCRIPTS = merge-logs

dist_bin_SCRIPTS = siptrace-merge

EXTRA_DIST = \
    $(bin_SCRIPTS:=.in)

$(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(bin_SCRIPTS)