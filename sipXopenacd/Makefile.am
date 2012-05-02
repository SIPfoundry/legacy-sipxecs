include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/project.am

SUBDIRS = \
	bin \
	etc \
	src

@PACKAGE@_EXTRA_DIST = \
	src \
	bin \
	etc \
	deps \
	$(REBAR) \
	$(REBAR).config \
	README \
	LICENSE

REBAR=./rebar
TARBALL = $(abspath $(DESTDIR)sipxopenacd-$(VERSION).tar.gz)

erlang_src = $(srcdir)/src
rebar= $(srcdir)/rebar \
	$(srcdir)/rebar.config
OPENACD_VARS = \
	$(filter OPENACD%,$(.VARIABLES))
dist_EXTRA_REPLACE = $(OPENACD_VARS)

noinst_SCRIPTS = sipxopenacd.spec

all: deps rcompile 

deps:
	$(shell cp -r $(erlang_src) $(abs_builddir) )
	$(shell cp -r $(rebar) $(abs_builddir) )
	$(REBAR) get-deps update-deps
	rm -f deps/OpenACD/openacd.spec
	rm -f deps/protobuffs/cover.spec
	rm -f deps/erlmongo/erlmongo.spec

rcompile:
	$(shell cp -r $(erlang_src) $(abs_builddir) )
	$(shell cp -r $(rebar) $(abs_builddir) )
	ERL_LIBS=@OPENACD_DIR@/lib $(REBAR) compile skip_deps=true

clean:
	$(REBAR) clean

dist: deps

rpm: dist
	QA_RPATHS=0x0003 rpmbuild -tb $(TARBALL)

.PHONY: all deps rcompile clean install dist rpm sipxopenacd.spec