
include $(top_srcdir)/config/subdir.am
include ant-targets.am

INSTALLER_FILES = \
	 installer/README.html \
	 installer/LICENSES \
	 installer/icons/logo.gif \
	 installer/icons/logo.png \
	 installer/icons/logo-sipfoundry.jpg \
	 installer/icons/sipxconfig-logo.png \
	 installer/startup/sipviewer \
	 installer/startup/start-sipviewer.bat \
	 installer/tools/izpack-install.xml \
	 installer/tools/ProcessPanelSpec.xml \
	 installer/tools/standalone-compiler.jar \
	 installer/tools/windows-izpack-shortcut.xml

EXTRA_DIST = \
    ant-targets.xml \
    ant-targets.am \
    config.properties.in \
	 build.xml \
	 src \
	 $(INSTALLER_FILES) \
	 bin

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/config.properties.in > config.properties

