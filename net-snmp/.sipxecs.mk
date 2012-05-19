net-snmp_VER = 5.7.1
net-snmp_REV = 100
net-snmp_SPEC = $(SRC)/$(PROJ)/net-snmp.spec
net-snmp_SRPM = net-snmp-$(net-snmp_VER)-$(net-snmp_REV)$(RPM_DIST).src.rpm
net-snmp_RPM_DEFS = --define='netsnmp_check 0' --define='_unitdir /lib/systemd/system'
net-snmp_FILES = \
	net-snmp-5.5-apsl-copying.patch \
	net-snmp-5.5-dir-fix.patch \
	net-snmp-5.5-include-struct.patch \
	net-snmp-5.5-perl-linking.patch \
	net-snmp-5.6.1-mysql.patch \
	net-snmp-5.6-multilib.patch \
	net-snmp-5.6-pie.patch \
	net-snmp-5.6-test-debug.patch \
	net-snmp-5.7.1-systemd.patch \
	net-snmp-5.7-libtool.patch \
	net-snmp-5.7-mibs-perl-linking.patch \
	0001-Support-for-listing-processes-specified-in-ucd-snmp-.patch \
	0002-autotools-generated-output-for-pcre-fix.patch \
	net-snmp-config \
	net-snmp-config.h \
	net-snmpd.init \
	net-snmpd.sysconfig \
	net-snmp.redhat.conf \
	net-snmp-tmpfs.conf \
	net-snmptrapd.init \
	net-snmp-trapd.redhat.conf \
	net-snmptrapd.sysconfig \
	snmpd.service \
	snmptrapd.service

net-snmp_SOURCES = \
	net-snmp-$(net-snmp_VER).tar.gz \
	$(addprefix $(SRC)/$(PROJ)/,$(net-snmp_FILES))

# targets not defined, nothing to do
net-snmp.autoreconf net-snmp.configure net-snmp.dist :;
