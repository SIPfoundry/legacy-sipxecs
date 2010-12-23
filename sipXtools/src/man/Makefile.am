# Parallel build cause docbook to choke
#   ../manpages/docbook.xsl sipdb-dump.xml
#   ../dbpoolx.mod:3899: parser warning : PEReference: %tbl.frame.attval; not found
#		frame		(%tbl.frame.attval;)	#IMPLIED
# So disable them here for all targets in this file
.NOTPARALLEL:

if GENERATE_MANPAGES
MAN_PAGES_OPT = $(MAN_PAGES)
endif

man_MANS = $(MAN_PAGES_OPT)

MAN_PAGES = \
	sipx-stats.1 \
	sipx-alias2dot.1 \
	sipx-readroute.1 \
	sipx-trace.1 \
	sipx-log-cut.1 \
	sipx-network-trace.1 \
	sipx-servtest.1 \
	expire-spread.1 \
	monitor-spread.1 \
	regtimes.1 \
	sipxcar.1 \
	sipxproc.1 \
	polycom-config.1

# Call me crazy, but this man page is causing occasional build failures, taking out for now
# and this will go away when FreeDB goes away
#	sipdb-dump.1

EXTRA_DIST = \
	$(subst .1,.txt2man,$(MAN_PAGES))

%.xml : %.txt2man
	$(ASCIIDOC) -d manpage -b docbook -o $@ $<

%.1 : %.xml
	$(XSLTPROC) $(DOCBOOK_2_MAN_XSL) $<
