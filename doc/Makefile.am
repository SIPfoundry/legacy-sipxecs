RPM_TARGET_ARCH=noarch

include config/project.am

RPMS = $(RPM)

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

CLEANFILES=

if DOC

DOCBOOK_XML_DOCS = \
	Hotelling \
	ClusterManagement

include config/docbook.am

$(foreach out,$(OUTPUT_FORMS),ClusterManagement.$(out)) : ClusterManagement.% : process_state.svg

$(foreach out,$(OUTPUT_FORMS),ClusterManagement.$(out)) : ClusterManagement.% : process_state.png

OTHER_DOCS+=process_state.svg process_state.png
CLEANFILES+=process_state.svg process_state.png

EXTRA_DIST+=process_state.dot

endif # DOC

all: $(DOCBOOK_OUTPUTS) $(OTHER_DOCS)

install: $(foreach doc,$(DOCBOOK_OUTPUTS) $(OTHER_DOCS),$(DESTDIR)@SIPX_DOCDIR@/$(doc))

$(foreach doc,$(DOCBOOK_OUTPUTS) $(OTHER_DOCS),$(DESTDIR)@SIPX_DOCDIR@/$(doc)) : $(DESTDIR)@SIPX_DOCDIR@/% : %
	$(INSTALL) -D -m 644 $< $@;
