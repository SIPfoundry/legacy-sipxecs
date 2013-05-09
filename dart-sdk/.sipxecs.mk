#
# You must install FPM by running
#    gem install fpm
#
lib_all += dart-sdk

dart-sdk_VER = 0.5.5
dart-sdk_REL = 22416
dart-sdk_TARBALL = dartsdk-linux-$(DISTRO_ARCH)-$(dart-sdk_VER)-r$(dart-sdk_REL).tar.gz
dart-sdk_i386_RPM = dart-sdk-$(dart-sdk_VER)-$(dart-sdk_REL).i386.rpm
dart-sdk_x86_64_RPM = dart-sdk-$(dart-sdk_VER)-$(dart-sdk_REL).x86_64.rpm
dart-sdk_intermediate_RPM = dart-sdk-$(dart-sdk_VER)-$(dart-sdk_REL).noarch.rpm

dart-sdk.dist dart-sdk.srpm :;

# separate copy step is needed because LHS of rpm target cannot include $(MOCK_RESULTS_DIR)
# because it's defined AFTER this make file is loaded.
dart-sdk.rpm_ : $(dart-sdk_$(DISTRO_ARCH)_RPM);
	cp -p $< $(MOCK_RESULTS_DIR)

$(dart-sdk_x86_64_RPM) : $(DOWNLOAD_LIB_CACHE)/$(dart-sdk_TARBALL)
	fpm -f -s tar -t rpm -n dart-sdk -v $(dart-sdk_VER) --iteration $(dart-sdk_REL) \
	  --after-install $(SRC)/dart-sdk/after-install.sh \
	  --license BSD --prefix /opt -a $(DISTRO_ARCH) $<

# I cannot generate 32 bit rpm on 64 bit machine because fpm tool will not let me.
# Yes this assumes build host machine is 64bit
$(dart-sdk_i386_RPM) : $(DOWNLOAD_LIB_CACHE)/$(dart-sdk_TARBALL)
	fpm -f -s tar -t rpm -n dart-sdk -v $(dart-sdk_VER) --iteration $(dart-sdk_REL) \
	  --after-install $(SRC)/$(PROJ)/after-install.sh \
	  --license BSD --prefix /opt -a noarch $<
	mv $(dart-sdk_intermediate_RPM) $@
