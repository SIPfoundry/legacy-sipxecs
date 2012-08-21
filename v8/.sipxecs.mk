v8_VER = 3.3.10
v8_REV = 4
v8_SRPM = v8-$(v8_VER)-$(v8_REV).el6.src.rpm

# targets not defined, nothing to do
v8.autoreconf v8.configure v8.dist:;
