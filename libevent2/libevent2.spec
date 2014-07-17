Name:          libevent2
Version:       2.0.21
Release:       1
License:       BSD-3-Clause
Group:         Development/Libraries/C and C++
URL:           http://libevent.org/
Summary:       An event notification library

Source: %name-%version.tar.gz

Prefix: %_prefix
BuildRoot: %{_tmppath}/%name-%version-root

BuildRequires: libtool automake autoconf

%description
The libevent API provides a mechanism to execute a callback function when
a specific event occurs on a file descriptor or after a timeout has been
reached. Furthermore, libevent also support callbacks due to signals or regular
timeouts.

libevent is meant to replace the event loop found in event driven network
servers. An application just needs to call event_dispatch() and then add or
remove events dynamically without having to change the event loop.

Currently, libevent supports /dev/poll, kqueue(2), event ports, select(2),
poll(2) and epoll(4). The internal event mechanism is completely independent of
the exposed event API, and a simple update of libevent can provide new
functionality without having to redesign the applications. As a result, Libevent
allows for portable application development and provides the most scalable event
notification mechanism available on an operating system. Libevent can also be
used for multi-threaded applications; see Steven Grimm's explanation. Libevent
should compile on Linux, *BSD, Mac OS X, Solaris and Windows.

%package -n    libevent2-devel
Group:         Development/Languages/C and C++
Summary:       Development files for %{name}
Requires:      %{name} = %{version}
Provides:      pkgconfig(libevent)

%description -n libevent2-devel
Development files for %{name}

# Preparation step (unpackung and patching if necessary)
%prep
%setup -q

%build
./autogen.sh
%configure --includedir=%{_includedir}/libevent2 --bindir=%{_bindir} --libdir=%{_libdir} \
 --disable-static
%__make %{?_smp_mflags}

%install
%__make install DESTDIR=%{buildroot}

%clean
[ "%{buildroot}" != "/" ] && rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%changelog

%files
%defattr(-,root,root)
%doc LICENSE README
%{_libdir}/libevent*.so.*

# Development stuff
%files -n libevent2-devel
%defattr(-,root,root)
%{_bindir}/event2_rpcgen.py
%{_libdir}/pkgconfig/*.pc
%{_includedir}/libevent2/*.h
%{_includedir}/libevent2/event2/
%{_libdir}/libevent*.so
%exclude %{_libdir}/libevent*.la
