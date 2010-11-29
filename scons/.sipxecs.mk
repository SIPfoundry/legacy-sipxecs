scons_SRPM = scons-0.98.0-1$(RPM_DIST).src.rpm
scons_SPEC = $(SRC)/$(PROJ)/scons.spec
scons_SOURCES = $(SRC)/$(PROJ)/scons-0.98.0.tar.gz

# targets not defined, nothing to do
scons.autoreconf scons.configure scons.dist:;

