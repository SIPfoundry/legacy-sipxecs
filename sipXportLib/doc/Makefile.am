include $(top_srcdir)/config/subdir.am
include $(top_srcdir)/config/doxygen.am

images = \
	PluginUml.png

EXTRA_DIST = \
  Doxyfile.in \
  footer.html \
  $(images)

doc: external-docs

.PHONY: external-docs
external-docs: @PACKAGE@ $(foreach img,$(images),@PACKAGE@/$(img))

$(foreach img,$(images),@PACKAGE@/$(img)): @PACKAGE@/% : %
	cp $< $@
