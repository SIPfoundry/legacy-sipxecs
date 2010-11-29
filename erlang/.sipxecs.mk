erlang_SRPM = erlang-R13B04-1.1.src.rpm
erlang_SPEC = $(SRC)/$(PROJ)/erlang.spec
erlang_SOURCES = \
  $(SRC)/$(PROJ)/erlang-R13B04-rpmlintrc \
  $(SRC)/$(PROJ)/otp_doc_html_R13B04.tar.bz2 \
  $(SRC)/$(PROJ)/otp_doc_man_R13B04.tar.bz2 \
  $(SRC)/$(PROJ)/otp-R13B04-emacs.patch \
  $(SRC)/$(PROJ)/otp-R13B04-rpath.patch \
  $(SRC)/$(PROJ)/otp-R13B04-sslrpath.patch \
  $(SRC)/$(PROJ)/otp_src_R13B04.tar.bz2

# targets not defined, nothing to do
erlang.autoreconf erlang.configure erlang.dist:;

