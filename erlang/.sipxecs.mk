lib_centos_6 += erlang

erlang_SRPM=erlang-R15B-03.2.fc17.src.rpm

# targets not defined, nothing to do
erlang.autoreconf erlang.configure erlang.dist:;
