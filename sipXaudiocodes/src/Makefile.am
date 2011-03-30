JAR_FILE = sipxconfig-audiocodes.jar

EXTRA_DIST = org

all : $(JAR_FILE)

clean :
	! test -f $(JAR_FILE) || rm $(JAR_FILE)
	! test -d classes || rm -rf classes

jardir = @SIPX_JAVADIR@/sipXconfig

jar_DATA = $(JAR_FILE)

jar_SOURCE = $(shell find $(srcdir)/org -type f -name '*.java')

RUNTIME_CLASSPATH = @SIPX_JAVADIR@/sipXconfig/sipxconfig.jar

# safer to generate each time
.PHONY: $(JAR_FILE)
$(JAR_FILE) :
	test -d classes || mkdir classes
	javac -classpath $(RUNTIME_CLASSPATH) -sourcepath $(srcdir) -d classes $(jar_SOURCE)
	jar -cf $@.tmp -C classes .
	cd $(srcdir); jar -uf $(abspath .)/$@.tmp `find org -type f -not -name '*.java'`
	mv $@.tmp $@
