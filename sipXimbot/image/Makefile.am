image_path=@SIPX_VARDIR@/sipximbot/image

IMAGE_DIST = \
   avatar.jpg 

EXTRA_DIST = \
	$(IMAGE_DIST)

install-data-hook : install-image

.PHONY: install-image
install-image : $(foreach image,$(IMAGE_DIST),$(DESTDIR)$(image_path)/$(image))

$(foreach image,$(IMAGE_DIST),$(DESTDIR)$(image_path)/$(image)) : $(DESTDIR)$(image_path)/% : $(srcdir)/%
	$(INSTALL) -D -m 644 $< $@
