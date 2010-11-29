rubygems_SRPM = rubygems-1.2.0-2.src.rpm
rubygems_SPEC = $(SRC)/$(PROJ)/rubygems.spec
rubygems_SOURCES = $(SRC)/$(PROJ)/rubygems-1.2.0.tgz

# targets not defined, nothing to do
rubygems.autoreconf rubygems.configure rubygems.dist:;

