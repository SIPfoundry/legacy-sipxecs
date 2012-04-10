
# needed files but not installed as part of net-snmp-devel
EXTRA_DIST = \
	errormib.h \
	kernel.h \
	mibdefs.h \
	regexp_proc.h \
	swrun.h

snmpdir = $(SIPX_LIBDIR)/snmpagent
snmp_LTLIBRARIES = regexp_proc.la

regexp_proc_la_CFLAGS = \
	$(shell net-snmp-config --cflags)

regexp_proc_la_LDFLAGS = \
	$(shell net-snmp-config --libs) \
	-lpcre \
	-module \
    -shared \
	-fPIC

dist_regexp_proc_la_SOURCES = \
	regexp_proc.c
