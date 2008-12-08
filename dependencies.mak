# this is used by sipxbuild to compute a correct build order

.PHONY: doc
doc : sipXcommserverLib
	@echo doc

.PHONY: sipXtools
sipXtools : sipXcommons
	@echo sipXtools

.PHONY: sipXbuild
sipXbuild :
	@echo sipXbuild

.PHONY: sipXportLib
sipXportLib :
	@echo sipXportLib

.PHONY: sipXtackLib
sipXtackLib : sipXportLib
	@echo sipXtackLib

.PHONY: sipXmediaLib
sipXmediaLib : sipXtackLib
	@echo sipXmediaLib

.PHONY: sipXmediaAdapterLib
sipXmediaAdapterLib : sipXmediaLib
	@echo sipXmediaAdapterLib

.PHONY: sipXcallLib
sipXcallLib : sipXmediaAdapterLib
	@echo sipXcallLib

.PHONY: sipXcommserverLib
sipXcommserverLib : sipXtackLib
	@echo sipXcommserverLib

.PHONY: sipXcommons
sipXcommons :
	@echo sipXcommons

.PHONY: sipXpublisher
sipXpublisher : sipXcommserverLib
	@echo sipXpublisher

.PHONY: sipXregistry
sipXregistry : sipXcommserverLib
	@echo sipXregistry

.PHONY: sipXproxy
sipXproxy : sipXcommserverLib
	@echo sipXproxy

.PHONY: sipXconfig
sipXconfig : sipXcommons
	@echo sipXconfig

.PHONY: sipXvxml
sipXvxml : sipXcallLib sipXcommserverLib sipXmediaAdapterLib
	@echo sipXvxml

.PHONY: sipXacd
sipXacd : sipXcallLib
	@echo sipXacd

.PHONY: sipXpbx
sipXpbx : sipXproxy sipXregistry sipXpublisher sipXvxml sipXconfig
	@echo sipXpbx

.PHONY: sipXsupervisor
sipXsupervisor : sipXcommserverLib sipXpbx
	@echo sipXsupervisor

.PHONY: sipXbridge
sipXbridge : sipXcommons
	@echo sipXbridge

.PHONY: sipXpage
sipXpage : sipXcommons
	@echo sipXpage

.PHONY: sipXivr
sipXivr : sipXcommons
	@echo sipXivr

.PHONY: sipXecs
sipXecs : sipXproxy sipXregistry sipXpublisher sipXvxml sipXconfig sipXpbx sipXsupervisor doc
	@echo sipXecs
