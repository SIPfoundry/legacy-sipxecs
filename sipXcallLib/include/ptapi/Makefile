PERCEPS=perl "z:/swdev/tools/perceps/perceps.pl"
DOCTEMPLATES="z:/swdev/tools/perceps/templates/api/PDFtemplates"
DOCOUTPUT=html
CLASS=//z/swdev/tools/perceps/classes

all install:
	$(PERCEPS) -a -h -f -m -d $(DOCOUTPUT) -t $(DOCTEMPLATES) \
            .
	cp $(CLASS)/*.class $(DOCOUTPUT)

clean:
	@ rm -f $(DOCOUTPUT)/.perceps
	@ rm -f $(DOCOUTPUT)/*.html
	@ rm -f $(DOCOUTPUT)/*.class
	@ rm -f $(DOCOUTPUT)/*.mcf
