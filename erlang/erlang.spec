# norootforbuild

Name:           erlang
Version:        R13B04
Release:        1.2
%define pkg_version R13B04
#
Group:          Development/Languages/Erlang
License:        Erlang Public License
#
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  ncurses-devel openssl-devel unixODBC-devel tcl-devel tk-devel openssh
BuildRequires:  gcc-c++ java-devel >= 1.5.0
BuildRequires:  krb5-devel 
%if 0%{?sles_ersion} != 0
BuildRequires:  Mesa-devel
%else 
BuildRequires:  mesa-libOSMesa-devel
%endif
%if 0%{?sles_version} >= 10
BuildRequires:  update-alternatives
%endif
%if 0%{?suse_version} > 1020
BuildRequires:  fdupes
%endif

#
URL:            http://www.erlang.org
Source:         otp_src_%{pkg_version}.tar.bz2
Source1:        otp_doc_html_%{pkg_version}.tar.bz2
Source2:        otp_doc_man_%{pkg_version}.tar.bz2
Source3:        %{name}-%{version}-rpmlintrc
Patch0:         otp-R13B04-rpath.patch
Patch1:         otp-R13B04-sslrpath.patch
Patch2:         otp-R13B04-emacs.patch
#
Summary:        General-purpose programming language and runtime environment
%description
Erlang is a general-purpose programming language and runtime
environment. Erlang has built-in support for concurrency, distribution
and fault tolerance. Erlang is used in several large telecommunication
systems from Ericsson.


%package debugger
Summary:	A debugger for debugging and testing of Erlang programs
Group:		Development/Languages/Erlang
Requires:       %{name} = %{version}-%{release}
Requires:       %{name}-gs = %{version}-%{release}
Requires:       %{name}-wx = %{version}-%{release}

%description debugger
A debugger for debugging and testing of Erlang programs.

%package dialyzer
Summary:	A DIscrepany AnaLYZer for ERlang programs
Group:		Development/Languages/Erlang
Requires:       %{name} = %{version}-%{release}
Requires:       %{name}-gs = %{version}-%{release}
Requires:       %{name}-wx = %{version}-%{release}

%description dialyzer
A DIscrepany AnaLYZer for ERlang programs.

%package jinterface
Summary:    Erlang Java Interface
Group:      Development/Libraries/Java
Requires:   %{name} == %{version}-%{release}, java >= 1.5.0

%description jinterface
JInterface module for accessing erlang from Java


%prep
rm -rf $RPM_BUILD_ROOT
%setup -q -n otp_src_%{pkg_version}
%patch0 -p1 -b .rpath
%patch1 -p1 -b .sslrpath
%patch2 -p1 -b .emacs

chmod -R u+w .
# enable dynamic linking for ssl
sed -i 's|SSL_DYNAMIC_ONLY=no|SSL_DYNAMIC_ONLY=yes|' erts/configure
# Remove shipped zlib sources
#rm -f erts/emulator/zlib/*.[ch]

# fix for arch linux bug #17001 (wx not working)
# sed -i 's|WX_LIBS=`$WX_CONFIG_WITH_ARGS --libs`|WX_LIBS="`$WX_CONFIG_WITH_ARGS --libs` -lGLU"|' lib/wx/configure || return 1

%build
# we need build only 1.5 target for java
# for SLE only
%if 0%{?sles_version} >= 10 || 0%{?suse_version} >= 1110
	export JAVAC="javac -target 1.5"
%endif
%if 0%{?suse_version} == 1100 || 0%{?fedora_version} == 9
export CFLAGS="-fno-strict-aliasing"
%else
export CFLAGS="$RPM_OPT_FLAGS -fno-strict-aliasing"
%endif
export CXXFLAGS=$CFLAGS

%configure \
    --with-ssl=%{_prefix} \
    --enable-threads \
    --enable-smp-support \
    --enable-kernel-poll \
    --enable-hipe \
    --enable-shared-zlib

%{__make}
# parallel builds do not work (yet)
# make %{?jobs:-j%jobs}


%install
make DESTDIR=$RPM_BUILD_ROOT install

# clean up gui related folders
rm -rf $RPM_BUILD_ROOT%{_libdir}/erlang/lib/et-1.4
rm -rf $RPM_BUILD_ROOT%{_libdir}/erlang/lib/gs-1.5.11
rm -rf $RPM_BUILD_ROOT%{_libdir}/erlang/lib/reltool-0.5.3
rm -rf $RPM_BUILD_ROOT%{_libdir}/erlang/lib/toolbar-1.4.1
rm -rf $RPM_BUILD_ROOT%{_libdir}/erlang/lib/wx-0.98.5

# clean up
find $RPM_BUILD_ROOT%{_libdir}/erlang -perm 0775 | xargs chmod -v 0755
find $RPM_BUILD_ROOT%{_libdir}/erlang -name Makefile | xargs chmod -v 0644
find $RPM_BUILD_ROOT%{_libdir}/erlang -name \*.bat | xargs rm -fv
find $RPM_BUILD_ROOT%{_libdir}/erlang -name index.txt.old | xargs rm -fv
rm $RPM_BUILD_ROOT%{_libdir}/erlang/lib/tools-2.6.5.1/emacs/test.erl.orig
mv $RPM_BUILD_ROOT%{_libdir}/erlang/lib/tools-2.6.5.1/emacs/test.erl.indented $RPM_BUILD_ROOT%{_libdir}/erlang/lib/tools-2.6.5.1/emacs/test.erl

# doc
mv README.md README
mkdir -p erlang_doc
tar -C erlang_doc -xjf %{SOURCE1}
tar -C $RPM_BUILD_ROOT/%{_libdir}/erlang -xjf %{SOURCE2}
# compress man pages ...
find %{buildroot}%{_libdir}/erlang/man -type f -exec gzip {} +

# use the links made by 'make install' instead of linking here
## make links to binaries
#mkdir -p $RPM_BUILD_ROOT/%{_bindir}
#cd $RPM_BUILD_ROOT/%{_bindir}
#for file in erl erlc
#do
#  ln -sf ../%{_lib}/erlang/bin/$file .
#done

# cleanup unused sources
# find $RPM_BUILD_ROOT%{_libdir}/erlang/lib -maxdepth 2 -type d -name *src -exec rm -rf {} \;

#make link to OtpErlang-*.jar in %{_javadir}
mkdir -p $RPM_BUILD_ROOT%{_javadir}
cd $RPM_BUILD_ROOT%{_javadir}
export JINTERFACE_VERSION=`ls $RPM_BUILD_ROOT%{_libdir}/erlang/lib/ |grep jinterface | sed "s|jinterface-||"`
ln -sf ../../%{_lib}/erlang/lib/jinterface-$JINTERFACE_VERSION/priv/OtpErlang.jar OtpErlang-$JINTERFACE_VERSION.jar
cd -

# emacs: automatically load support for erlang
# http://lists.mandriva.com//bugs/2007-08/msg00930.php
export TOOLS_VERSION=`ls $RPM_BUILD_ROOT%{_libdir}/erlang/lib/ |grep tools- | sed "s|tools-||"`
mkdir -p $RPM_BUILD_ROOT%{_datadir}/emacs/site-lisp
cat > $RPM_BUILD_ROOT%{_datadir}/emacs/site-lisp/erlang.el << EOF
(setq load-path (cons "%{_libdir}/erlang/lib/tools-$TOOLS_VERSION/emacs" load-path))
(add-to-list 'load-path "%{_datadir}/emacs/site-lisp/ess")
(load-library "erlang-start")
EOF

# automatically install common_test wrapper shell script
# this normally needs to be run in the installed directories
# -> do it inside build root here and remove any reference to the build root from the installed script
export CT_VERSION=`ls $RPM_BUILD_ROOT%{_libdir}/erlang/lib/ |grep common_test- | sed "s|common_test-||"`
$RPM_BUILD_ROOT/%{_libdir}/erlang/lib/common_test-$CT_VERSION/install.sh $RPM_BUILD_ROOT%{_libdir}/erlang/lib
sed -i "s|$RPM_BUILD_ROOT||" $RPM_BUILD_ROOT%{_libdir}/erlang/lib/common_test-$CT_VERSION/priv/bin/run_test


%if 0%{?suse_version} > 1020
# hardlink duplicates:
%fdupes $RPM_BUILD_ROOT/%{_libdir}/erlang
# %doc macro copies the files to the package doc dir, hardlinks thus don't work
%fdupes -s erlang_doc
%endif


%clean
rm -rf $RPM_BUILD_ROOT


%files 
%defattr(-,root,root)
%doc AUTHORS EPLICENCE README 
%doc %{_libdir}/erlang/PR.template
%doc %{_libdir}/erlang/README
%doc %{_libdir}/erlang/COPYRIGHT
%{_bindir}/*
%dir %{_libdir}/erlang
%dir %{_libdir}/erlang/lib/
%{_libdir}/erlang/bin/
%{_libdir}/erlang/erts-*/
%{_libdir}/erlang/lib/appmon-*/
%{_libdir}/erlang/lib/asn1-*/
%{_libdir}/erlang/lib/common_test-*/
%{_libdir}/erlang/lib/compiler-*/
%{_libdir}/erlang/lib/cosEvent-*/
%{_libdir}/erlang/lib/cosEventDomain-*/
%{_libdir}/erlang/lib/cosFileTransfer-*/
%{_libdir}/erlang/lib/cosNotification-*/
%{_libdir}/erlang/lib/cosProperty-*/
%{_libdir}/erlang/lib/cosTime-*/
%{_libdir}/erlang/lib/cosTransactions-*/
%{_libdir}/erlang/lib/crypto-*/
%{_libdir}/erlang/lib/docbuilder-*/
%{_libdir}/erlang/lib/edoc-*/
%{_libdir}/erlang/lib/erl_docgen-*/
%{_libdir}/erlang/lib/erl_interface-*/
%{_libdir}/erlang/lib/erts-*/
%{_libdir}/erlang/lib/eunit-*/
%{_libdir}/erlang/lib/hipe-*/
%{_libdir}/erlang/lib/ic-*/
%{_libdir}/erlang/lib/inets-*/
%{_libdir}/erlang/lib/inviso-*/
%{_libdir}/erlang/lib/kernel-*/
%{_libdir}/erlang/lib/megaco-*/
%{_libdir}/erlang/lib/mnesia-*/
%{_libdir}/erlang/lib/observer-*/
%{_libdir}/erlang/lib/odbc-*/
%{_libdir}/erlang/lib/orber-*/
%{_libdir}/erlang/lib/os_mon-*/
%{_libdir}/erlang/lib/otp_mibs-*/
%{_libdir}/erlang/lib/parsetools-*/
%{_libdir}/erlang/lib/percept-*/
%{_libdir}/erlang/lib/pman-*/
%{_libdir}/erlang/lib/public_key-*/
%{_libdir}/erlang/lib/runtime_tools-*/
%{_libdir}/erlang/lib/sasl-*/
%{_libdir}/erlang/lib/snmp-*/
%{_libdir}/erlang/lib/ssh-*/
%{_libdir}/erlang/lib/ssl-*/
%{_libdir}/erlang/lib/stdlib-*/
%{_libdir}/erlang/lib/syntax_tools-*/
%{_libdir}/erlang/lib/test_server-*/
%{_libdir}/erlang/lib/tools-*/
%{_libdir}/erlang/lib/tv-*/
%{_libdir}/erlang/lib/typer-*/
%{_libdir}/erlang/lib/webtool-*/
%{_libdir}/erlang/lib/xmerl-*/
%{_libdir}/erlang/man/
%{_libdir}/erlang/misc/
%{_libdir}/erlang/releases/
%{_libdir}/erlang/usr/
%{_libdir}/erlang/Install
%{_datadir}/emacs/site-lisp/erlang.el

%files debugger
%defattr(-,root,root)
%{_libdir}/erlang/lib/debugger-*/

%files dialyzer
%defattr(-,root,root)
%{_libdir}/erlang/lib/dialyzer-*/

%files jinterface
%defattr(-,root,root,-)
%{_libdir}/erlang/lib/jinterface*
%{_javadir}/*

%post
%{_libdir}/erlang/Install -minimal %{_libdir}/erlang >/dev/null 2>/dev/null


%changelog
