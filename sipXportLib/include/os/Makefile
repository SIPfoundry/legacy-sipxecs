PERCEPS=perl "z:/swdev/tools/perceps/perceps.pl"
DOCTEMPLATES="f:/Alison/ahodges_dev/PDFtemplates/PDFinternal"
DOCOUTPUT=html
CLASS=//z/swdev/tools/perceps/classes

all install:
	$(PERCEPS) -a -h -f -m -d $(DOCOUTPUT) -t $(DOCTEMPLATES) \
            .
	cp $(CLASS)/*.class $(DOCOUTPUT)

clean:
	rm -f $(DOCOUTPUT)/.perceps
	rm -f $(DOCOUTPUT)/*.html
	rm -f $(DOCOUTPUT)/*.class
	rm -f $(DOCOUTPUT)/*.mcf
