# This script is the commands listed in 'build-notes'.  It builds sipXvxml.

set -ex
echo ${ARCH:?needs to be set to the hardware architecture}
echo ${TMPDIR:?needs to be set to the directory containing the tar\'s, etc.}
echo ${INSTALL_PREFIX:?needs to be set to the installation directory}

export BUILD=$(pwd)
svn checkout http://scm.sipfoundry.org/rep/sipXvxml/
svn checkout http://scm.sipfoundry.org/rep/sipXcallLib/
svn checkout http://scm.sipfoundry.org/rep/sipXtackLib/
svn checkout http://scm.sipfoundry.org/rep/sipXmediaLib/
svn checkout http://scm.sipfoundry.org/rep/sipXportLib/
for d in sipX* ; do ( cd $d ; svn info ) ; done | grep -E '^$|URL:|Revision:'
cd $BUILD
tar -xzf $TMPDIR/xerces-c-current.tar.gz
cd $BUILD/xerces-c-src_2_5_0
export XERCESCROOT=$(pwd)
cd src/xercesc
autoconf
./runConfigure -p linux -n libwww -P ~/$ARCH
make
make install
cd $BUILD
tar -xzf /usr/src/redhat/SOURCES/w3c-libwww-5.4.0.tgz
cd w3c-libwww-5.4.0
CPPFLAGS="-I/usr/kerberos/include" ./configure --prefix=$INSTALL_PREFIX --with-ssl
make
make install
cd $BUILD
tar -xzf $TMPDIR/cppunit-1.10.2.tar.gz
cd cppunit-1.10.2
./configure --prefix=$INSTALL_PREFIX
make
make check
make install
cd $BUILD
tar -xzf $TMPDIR/glib-2.2.1.tar.gz
cd glib-2.2.1
./configure --prefix=$INSTALL_PREFIX
make
rm -f ~/$ARCH/include/glib.h ~/$ARCH/include/gmodule.h
make install
cd $BUILD/sipXportLib/main
export ACLOCAL="aclocal -I $INSTALL_PREFIX/share/aclocal"
autoreconf --install --force
CXX=g++ CPPFLAGS="-I$INSTALL_PREFIX/include -I$INSTALL_PREFIX/include/glib-2.0 -I$INSTALL_PREFIX/include/glib-2.0/include -I/usr/kerberos/include" LDFLAGS="-L$INSTALL_PREFIX/lib -lglib-2.0" PKG_CONFIG_PATH=$INSTALL_PREFIX/lib/pkgconfig ./configure --prefix=$INSTALL_PREFIX --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
make all
make check
make install
cd $BUILD/sipXtackLib/main
autoreconf --install --force
CXX=g++ CPPFLAGS="-I$INSTALL_PREFIX/include -I$INSTALL_PREFIX/include/glib-2.0 -I$INSTALL_PREFIX/include/glib-2.0/include -I/usr/kerberos/include" LDFLAGS="-L$INSTALL_PREFIX/lib -lglib-2.0" PKG_CONFIG_PATH=$INSTALL_PREFIX/lib/pkgconfig ./configure --prefix=$INSTALL_PREFIX --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib --disable-sipviewer
make all
make check
make install
cd $BUILD/sipXmediaLib/main
autoreconf --install --force
CXX=g++ CPPFLAGS="-I$INSTALL_PREFIX/include -I$INSTALL_PREFIX/include/glib-2.0 -I$INSTALL_PREFIX/include/glib-2.0/include -I/usr/kerberos/include" LDFLAGS="-L$INSTALL_PREFIX/lib -lglib-2.0" PKG_CONFIG_PATH=$INSTALL_PREFIX/lib/pkgconfig ./configure --prefix=$INSTALL_PREFIX --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
make
make install
cd $BUILD/sipXcallLib/main
autoreconf --install --force
CXX=g++ CPPFLAGS="-I$INSTALL_PREFIX/include -I$INSTALL_PREFIX/include/glib-2.0 -I$INSTALL_PREFIX/include/glib-2.0/include -I/usr/kerberos/include" LDFLAGS="-L$INSTALL_PREFIX/lib -lglib-2.0" PKG_CONFIG_PATH=$INSTALL_PREFIX/lib/pkgconfig ./configure --prefix=$INSTALL_PREFIX --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
make
make install
cd $BUILD/sipXvxml/main
autoreconf --install --force
CXX=g++ CPPFLAGS="-I$INSTALL_PREFIX/include -I$INSTALL_PREFIX/include/glib-2.0 -I$INSTALL_PREFIX/include/glib-2.0/include -I/usr/kerberos/include" LDFLAGS="-L$INSTALL_PREFIX/lib -lglib-2.0" PKG_CONFIG_PATH=$INSTALL_PREFIX/lib/pkgconfig ./configure --prefix=$INSTALL_PREFIX --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib --with-xerces=$INSTALL_PREFIX
make
make install
