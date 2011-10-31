# NOTE: not the latest scons because mongo 2.0.0 is incompatible w/scons 2.1.0
scons_VER = 1.0.1
scons_SRPM = scons-$(scons_VER)-1.src.rpm

# Had to edit custom spec to avoid error
#  Checking for unpackaged file(s): /usr/lib/rpm/check-files /var/tmp/scons-buildroot
#   RPM build errors:
#      File not found: /var/tmp/scons-buildroot/usr/lib/scons/scons-2.0.1.egg-info
#scons_SPEC = $(SRC)/$(PROJ)/scons.spec

#scons_SOURCES = $(scons_SRPM) scons-2.0.1.tar.gz

# otherwise, unpackages *.pyo files on rhel5
scons_RPM_DEFS = --define "__os_install_post %{nil}"

scons.autoreconf scons.configure scons.dist:;

