include config/project.am

DOCBOOK_XML_DOCS = \
	ClusterManagement

OTHER_DOCS = \
	coding-standard.html \
	EclipseCDTprojects.txt

OUTPUT_FORMS=

if DOC

if GENERATE_DOCBOOK_HTML

OUTPUT_FORMS+=.html
XSL_JADETEX=style.xsl

%.html: %.xml $(XSL_JADETEX)
	$(XSLTPROC) -o $@ $(srcdir)/$(XSL_JADETEX) $<

endif # GENERATE_DOCBOOK_HTML

if GENERATE_DOCBOOK_PDF

OUTPUT_FORMS+=.pdf

PDF_CONTROL_FILES= \
	docbook-html.xsl \
	docbook-jadetex.dsssl \
	docbook.mak

DSSSL_JADETEX=/usr/share/sgml/docbook/dsssl-stylesheets/print/docbook.dsl

PDF_INTERMEDIATE_FORMS=.tex .aux .log .out
PDF_INTERMEDIATES=$(foreach doc,$(DOCBOOK_XML_DOCS),$(foreach intform,$(PDF_INTERMEDIATE_FORMS),$(doc)$(intform)))

%.tex: %.xml $(DSSSL_JADETEX)
	$(OPENJADE) -t tex -d $(DSSSL_JADETEX) -c /usr/share/sgml/openjade/catalog \
		-o $@ /usr/share/sgml/docbook/dsssl-stylesheets/dtds/decls/xml.dcl $<

%.pdf %.out %.aux %.log: %.tex
	if [ -e prior.aux ]; then cp -pf prior.aux pprior.aux; fi
	f=$(shell basename $< .tex).aux; if [ -e $$f ]; then cp -pf $$f prior.aux; fi
	-$(PDFJADETEX) $< > pdfjadetex.log
	if ! cmp $(shell basename $< .tex).aux prior.aux > /dev/null 2>&1 && \
		! cmp $(shell basename $< .tex).aux pprior.aux > /dev/null 2>&1 && \
		expr $(MAKELEVEL) '<' 4 > /dev/null; then \
		rm -f $@; \
		echo "Found undefined references, compiling again..."; \
		$(MAKE) $@; \
	fi

endif # GENERATE_DOCBOOK_PDF

DOCBOOK_CONTROL_FILES = \
	$(XSL_JADETEX) \
	$(PDF_CONTROL_FILES)

endif # DOC

DOCBOOK_OUTPUTS=$(foreach doc,$(DOCBOOK_XML_DOCS),$(foreach outform,$(OUTPUT_FORMS),$(doc)$(outform)))

all: $(DOCBOOK_OUTPUTS) $(OTHER_DOCS)

install: install-docs

.PHONY: install-docs
install-docs: $(foreach doc,$(DOCBOOK_OUTPUTS) $(OTHER_DOCS),$(DESTDIR)@SIPX_DOCDIR@/$(doc))

$(foreach doc,$(DOCBOOK_OUTPUTS) $(OTHER_DOCS),$(DESTDIR)@SIPX_DOCDIR@/$(doc)) : $(DESTDIR)@SIPX_DOCDIR@/% : %
	$(INSTALL) -D -m 644 $< $@; 

CLEANFILES=$(PDF_INTERMEDIATES)

EXTRA_DIST= \
	$(DOCBOOK_XML_DOCS) \
	$(DOCBOOK_CONTROL_FILES) \
   config/sipX-config.in \
   config/sipX-buildstamp.cpp.in \
   config/sipX-buildstamp.h.in \
   config/svn-version \
   BUILDSTAMP \
   SVN-VERSION \
   sipxecs-doc.spec

RPM_TARGET_ARCH=noarch
