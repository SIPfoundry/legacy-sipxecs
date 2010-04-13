include $(top_srcdir)/ant-targets.am

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties cmcprov.properties

config.properties : config.properties.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

cmcprov.properties : cmcprov.properties.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
