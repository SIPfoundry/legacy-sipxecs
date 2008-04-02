include config/project.am

OTHER_DOCS = \
	coding-standard.html \
	EclipseCDTprojects.txt

EXTRA_DIST= \
	$(OTHER_DOCS) \
   config/sipX-config.in \
   config/sipX-buildstamp.cpp.in \
   config/sipX-buildstamp.h.in \
   config/svn-version \
   BUILDSTAMP \
   SVN-VERSION \
   sipxecs-doc.spec

sipxecs-doc.spec : sipxecs-doc.spec.in
	$(LocalizeSipXconfig) $(srcdir)/sipxecs-doc.spec.in > sipxecs-doc.spec

CLEANFILES=

if DOC

DOCBOOK_XML_DOCS = \
	ClusterManagement

include config/docbook.am

endif # DOC

all: $(DOCBOOK_OUTPUTS) $(OTHER_DOCS)

install: $(foreach doc,$(DOCBOOK_OUTPUTS) $(OTHER_DOCS),$(DESTDIR)@SIPX_DOCDIR@/$(doc))

$(foreach doc,$(DOCBOOK_OUTPUTS) $(OTHER_DOCS),$(DESTDIR)@SIPX_DOCDIR@/$(doc)) : $(DESTDIR)@SIPX_DOCDIR@/% : %
	$(INSTALL) -D -m 644 $< $@;

RPM_TARGET_ARCH=noarch
