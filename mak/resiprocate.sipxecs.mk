resiprocate_VER = 1.9.0~beta6
resiprocate_REV = 1
resiprocate_SRPM = resiprocate-$(resiprocate_VER)-$(resiprocate_REV)$(RPM_DIST).src.rpm
resiprocate_TAR = $(SRC)/resiprocate/resiprocate-$(resiprocate_VER).tar.gz
resiprocate_SOURCES = $(resiprocate_TAR)

resiprocate.dist : $(SRC)/resiprocate/.git
	cd $(SRC)/resiprocate; \
	  make dist

