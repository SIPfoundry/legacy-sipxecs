rubygem-rake_SRPM = rubygem-rake-0.8.7-1$(RPM_DIST).src.rpm
rubygem-rake_SPEC = $(SRC)/$(PROJ)/rubygem-rake.spec
rubygem-rake_SOURCES = $(SRC)/$(PROJ)/rake-0.8.7.gem

# targets not defined, nothing to do
rubygem-rake.autoreconf rubygem-rake.configure rubygem-rake.dist:;

