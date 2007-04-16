# this is used echo back projects to circumvent unneeded dependency managment
# imposed from top.mak.in. should be readdressed this at somepoint.
.PHONY: $(MAKECMDGOALS)
$(MAKECMDGOALS) :
	@echo $@
