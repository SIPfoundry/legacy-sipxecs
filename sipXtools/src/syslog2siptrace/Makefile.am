## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = syslog2siptrace

syslog2siptrace_LDADD = \
	@SIPXTACK_LIBS@

syslog2siptrace_SOURCES = main.cpp

bin_SCRIPTS = merge-logs siptrace-merge

EXTRA_DIST = \
    merge-logs \
    siptrace-merge \
    merge-logs.in

merge-logs: $(srcdir)/merge-logs.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/merge-logs.in > merge-logs
