unittest_SRPM = unittest-0.50-62.6.el5.src.rpm

unittest.autoreconf unittest.configure unittest.dist :;

unittest.srpm :
	$(call CopySourceFile,$(unittest_SRPM),$(MOCK_SRPM_DIR))

