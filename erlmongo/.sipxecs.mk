erlmongo_VER = 0.0.9
erlmongo_REV = 1
erlmongo_SPEC = $(SRC)/erlmongo/erlmongo.spec
erlmongo_SOURCES = $(erlmongo_TARBALL)
erlmongo_SRPM = erlmongo-$(erlmongo_VER)-$(erlmongo_REV)$(RPM_DIST).src.rpm
erlmongo_TARBALL = erlmongo-$(erlmongo_VER).tar.gz

erlmongo.dist:;

# Helpful for package maintainer when you want to pull in new changes from erlmongo project
erlmongo.update_dist :
	git clone git://github.com/wpntv/erlmongo.git
	cd erlmongo; \
	  ./configure; \
	  make dist
	mv erlmongo/erlmongo-*.tar.gz $(SRC)/erlmongo


