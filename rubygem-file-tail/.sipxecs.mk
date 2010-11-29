rubygem-file-tail_SRPM = rubygem-file-tail-1.0.4-1$(RPM_DIST).src.rpm
rubygem-file-tail_SPEC = $(SRC)/$(PROJ)/rubygem-file-tail.spec
rubygem-file-tail_SOURCES = $(SRC)/$(PROJ)/file-tail-1.0.4.gem

# targets not defined, nothing to do
rubygem-file-tail.autoreconf rubygem-file-tail.configure rubygem-file-tail.dist:;

