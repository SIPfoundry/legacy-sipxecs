sec_VER = 2.6.2
sec_SRPM = sec-$(sec_VER)-1$(RPM_DIST).src.rpm
sec_RPM_DEFS = --define='_unitdir /lib/systemd/system'
sec_SPEC = $(SRC)/$(PROJ)/sec.spec
sec_FILES = \
	amavisd.sec \
	bsd-general.sec \
	bsd-MONITOR.sec \
	bsd-mpd.sec \
	bsd-PHYSMOD.sec \
	bsd-USERACT.sec \
	cisco-syslog.sec \
	conf.README \
	cvs.sec \
	dameware.sec \
	hp-openview.sec \
	labrea.sec \
	pix-general.sec \
	pix-security.sec \
	pix-url.sec \
	portscan.sec \
	sec.logrotate\
	sec.service\
	sec.spec\
	sec.spec.bak\
	snortsam.sec\
	snort.sec\
	ssh-brute.sec\
	ssh.sec\
	vtund.sec\
	windows.sec

sec_SOURCES = \
	$(addprefix $(SRC)/$(PROJ)/,$(sec_FILES)) \
	sec-$(sec_VER).tar.gz

sec.dist:;
