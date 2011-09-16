include $(top_srcdir)/config/utility.am

confdir = \
	$(SIPX_CONFDIR)

dist_conf_DATA = \
	sipxexample.xml

precommit : validate-xml

validate-xml : $(dist_conf_DATA)
	xmllint --noout $^

CLEANFILES = $(conf_DATA)
