rsyslog_SRPM = rsyslog-2.0.6-1$(RPM_DIST).src.rpm
rsyslog_SPEC = $(SRC)/$(PROJ)/rsyslog.spec
rsyslog_SOURCES = \
	$(SRC)/$(PROJ)/rsyslog-2.0.6.tar.gz \
	$(SRC)/$(PROJ)/rsyslog-2.0.0-forwardMsg.patch \
	$(SRC)/$(PROJ)/rsyslog.init \
	$(SRC)/$(PROJ)/rsyslog.sysconfig \
	$(SRC)/$(PROJ)/syslog.log


# targets not defined, nothing to do
rsyslog.autoreconf rsyslog.configure rsyslog.dist:;

