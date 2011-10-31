%define name scons
%define version 2.0.1
%define release 1
%define _unpackaged_files_terminate_build 0

Summary: an Open Source software construction tool
Name: %{name}
Version: %{version}
Release: %{release}
Source0: %{name}-%{version}.tar.gz
#Copyright: The SCons Foundation
License: MIT, freely distributable
Group: Development/Tools
BuildRoot: %{_tmppath}/%{name}-buildroot
Prefix: %{_prefix}
BuildArchitectures: noarch
Vendor: Steven Knight <knight@scons.org>
Packager: Steven Knight <knight@scons.org>
Requires: python >= 2.4
Url: http://www.scons.org/

%description
SCons is an Open Source software construction tool--that is, a build
tool; an improved substitute for the classic Make utility; a better way
to build software.  SCons is based on the design which won the Software
Carpentry build tool design competition in August 2000.

SCons "configuration files" are Python scripts, eliminating the need
to learn a new build tool syntax.  SCons maintains a global view of
all dependencies in a tree, and can scan source (or other) files for
implicit dependencies, such as files specified on #include lines.  SCons
uses MD5 signatures to rebuild only when the contents of a file have
really changed, not just when the timestamp has been touched.  SCons
supports side-by-side variant builds, and is easily extended with user-
defined Builder and/or Scanner objects.

%prep
%setup

%build
python setup.py build

%install
python setup.py install --root=$RPM_BUILD_ROOT --record=INSTALLED_FILES --install-lib=/usr/lib/scons --install-scripts=/usr/bin --install-data=/usr/share

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/bin/scons
/usr/bin/scons-2.0.1
/usr/bin/scons-time
/usr/bin/scons-time-2.0.1
/usr/bin/sconsign
/usr/bin/sconsign-2.0.1
/usr/lib/scons/SCons/Action.py
/usr/lib/scons/SCons/Action.pyc
/usr/lib/scons/SCons/Builder.py
/usr/lib/scons/SCons/Builder.pyc
/usr/lib/scons/SCons/CacheDir.py
/usr/lib/scons/SCons/CacheDir.pyc
/usr/lib/scons/SCons/Conftest.py
/usr/lib/scons/SCons/Conftest.pyc
/usr/lib/scons/SCons/Debug.py
/usr/lib/scons/SCons/Debug.pyc
/usr/lib/scons/SCons/Defaults.py
/usr/lib/scons/SCons/Defaults.pyc
/usr/lib/scons/SCons/Environment.py
/usr/lib/scons/SCons/Environment.pyc
/usr/lib/scons/SCons/Errors.py
/usr/lib/scons/SCons/Errors.pyc
/usr/lib/scons/SCons/Executor.py
/usr/lib/scons/SCons/Executor.pyc
/usr/lib/scons/SCons/Job.py
/usr/lib/scons/SCons/Job.pyc
/usr/lib/scons/SCons/Memoize.py
/usr/lib/scons/SCons/Memoize.pyc
/usr/lib/scons/SCons/Node/Alias.py
/usr/lib/scons/SCons/Node/Alias.pyc
/usr/lib/scons/SCons/Node/FS.py
/usr/lib/scons/SCons/Node/FS.pyc
/usr/lib/scons/SCons/Node/Python.py
/usr/lib/scons/SCons/Node/Python.pyc
/usr/lib/scons/SCons/Node/__init__.py
/usr/lib/scons/SCons/Node/__init__.pyc
/usr/lib/scons/SCons/Options/BoolOption.py
/usr/lib/scons/SCons/Options/BoolOption.pyc
/usr/lib/scons/SCons/Options/EnumOption.py
/usr/lib/scons/SCons/Options/EnumOption.pyc
/usr/lib/scons/SCons/Options/ListOption.py
/usr/lib/scons/SCons/Options/ListOption.pyc
/usr/lib/scons/SCons/Options/PackageOption.py
/usr/lib/scons/SCons/Options/PackageOption.pyc
/usr/lib/scons/SCons/Options/PathOption.py
/usr/lib/scons/SCons/Options/PathOption.pyc
/usr/lib/scons/SCons/Options/__init__.py
/usr/lib/scons/SCons/Options/__init__.pyc
/usr/lib/scons/SCons/PathList.py
/usr/lib/scons/SCons/PathList.pyc
/usr/lib/scons/SCons/Platform/__init__.py
/usr/lib/scons/SCons/Platform/__init__.pyc
/usr/lib/scons/SCons/Platform/aix.py
/usr/lib/scons/SCons/Platform/aix.pyc
/usr/lib/scons/SCons/Platform/cygwin.py
/usr/lib/scons/SCons/Platform/cygwin.pyc
/usr/lib/scons/SCons/Platform/darwin.py
/usr/lib/scons/SCons/Platform/darwin.pyc
/usr/lib/scons/SCons/Platform/hpux.py
/usr/lib/scons/SCons/Platform/hpux.pyc
/usr/lib/scons/SCons/Platform/irix.py
/usr/lib/scons/SCons/Platform/irix.pyc
/usr/lib/scons/SCons/Platform/os2.py
/usr/lib/scons/SCons/Platform/os2.pyc
/usr/lib/scons/SCons/Platform/posix.py
/usr/lib/scons/SCons/Platform/posix.pyc
/usr/lib/scons/SCons/Platform/sunos.py
/usr/lib/scons/SCons/Platform/sunos.pyc
/usr/lib/scons/SCons/Platform/win32.py
/usr/lib/scons/SCons/Platform/win32.pyc
/usr/lib/scons/SCons/SConf.py
/usr/lib/scons/SCons/SConf.pyc
/usr/lib/scons/SCons/SConsign.py
/usr/lib/scons/SCons/SConsign.pyc
/usr/lib/scons/SCons/Scanner/C.py
/usr/lib/scons/SCons/Scanner/C.pyc
/usr/lib/scons/SCons/Scanner/D.py
/usr/lib/scons/SCons/Scanner/D.pyc
/usr/lib/scons/SCons/Scanner/Dir.py
/usr/lib/scons/SCons/Scanner/Dir.pyc
/usr/lib/scons/SCons/Scanner/Fortran.py
/usr/lib/scons/SCons/Scanner/Fortran.pyc
/usr/lib/scons/SCons/Scanner/IDL.py
/usr/lib/scons/SCons/Scanner/IDL.pyc
/usr/lib/scons/SCons/Scanner/LaTeX.py
/usr/lib/scons/SCons/Scanner/LaTeX.pyc
/usr/lib/scons/SCons/Scanner/Prog.py
/usr/lib/scons/SCons/Scanner/Prog.pyc
/usr/lib/scons/SCons/Scanner/RC.py
/usr/lib/scons/SCons/Scanner/RC.pyc
/usr/lib/scons/SCons/Scanner/__init__.py
/usr/lib/scons/SCons/Scanner/__init__.pyc
/usr/lib/scons/SCons/Script/Interactive.py
/usr/lib/scons/SCons/Script/Interactive.pyc
/usr/lib/scons/SCons/Script/Main.py
/usr/lib/scons/SCons/Script/Main.pyc
/usr/lib/scons/SCons/Script/SConsOptions.py
/usr/lib/scons/SCons/Script/SConsOptions.pyc
/usr/lib/scons/SCons/Script/SConscript.py
/usr/lib/scons/SCons/Script/SConscript.pyc
/usr/lib/scons/SCons/Script/__init__.py
/usr/lib/scons/SCons/Script/__init__.pyc
/usr/lib/scons/SCons/Sig.py
/usr/lib/scons/SCons/Sig.pyc
/usr/lib/scons/SCons/Subst.py
/usr/lib/scons/SCons/Subst.pyc
/usr/lib/scons/SCons/Taskmaster.py
/usr/lib/scons/SCons/Taskmaster.pyc
/usr/lib/scons/SCons/Tool/386asm.py
/usr/lib/scons/SCons/Tool/386asm.pyc
/usr/lib/scons/SCons/Tool/BitKeeper.py
/usr/lib/scons/SCons/Tool/BitKeeper.pyc
/usr/lib/scons/SCons/Tool/CVS.py
/usr/lib/scons/SCons/Tool/CVS.pyc
/usr/lib/scons/SCons/Tool/FortranCommon.py
/usr/lib/scons/SCons/Tool/FortranCommon.pyc
/usr/lib/scons/SCons/Tool/JavaCommon.py
/usr/lib/scons/SCons/Tool/JavaCommon.pyc
/usr/lib/scons/SCons/Tool/MSCommon/__init__.py
/usr/lib/scons/SCons/Tool/MSCommon/__init__.pyc
/usr/lib/scons/SCons/Tool/MSCommon/arch.py
/usr/lib/scons/SCons/Tool/MSCommon/arch.pyc
/usr/lib/scons/SCons/Tool/MSCommon/common.py
/usr/lib/scons/SCons/Tool/MSCommon/common.pyc
/usr/lib/scons/SCons/Tool/MSCommon/netframework.py
/usr/lib/scons/SCons/Tool/MSCommon/netframework.pyc
/usr/lib/scons/SCons/Tool/MSCommon/sdk.py
/usr/lib/scons/SCons/Tool/MSCommon/sdk.pyc
/usr/lib/scons/SCons/Tool/MSCommon/vc.py
/usr/lib/scons/SCons/Tool/MSCommon/vc.pyc
/usr/lib/scons/SCons/Tool/MSCommon/vs.py
/usr/lib/scons/SCons/Tool/MSCommon/vs.pyc
/usr/lib/scons/SCons/Tool/Perforce.py
/usr/lib/scons/SCons/Tool/Perforce.pyc
/usr/lib/scons/SCons/Tool/PharLapCommon.py
/usr/lib/scons/SCons/Tool/PharLapCommon.pyc
/usr/lib/scons/SCons/Tool/RCS.py
/usr/lib/scons/SCons/Tool/RCS.pyc
/usr/lib/scons/SCons/Tool/SCCS.py
/usr/lib/scons/SCons/Tool/SCCS.pyc
/usr/lib/scons/SCons/Tool/Subversion.py
/usr/lib/scons/SCons/Tool/Subversion.pyc
/usr/lib/scons/SCons/Tool/__init__.py
/usr/lib/scons/SCons/Tool/__init__.pyc
/usr/lib/scons/SCons/Tool/aixc++.py
/usr/lib/scons/SCons/Tool/aixc++.pyc
/usr/lib/scons/SCons/Tool/aixcc.py
/usr/lib/scons/SCons/Tool/aixcc.pyc
/usr/lib/scons/SCons/Tool/aixf77.py
/usr/lib/scons/SCons/Tool/aixf77.pyc
/usr/lib/scons/SCons/Tool/aixlink.py
/usr/lib/scons/SCons/Tool/aixlink.pyc
/usr/lib/scons/SCons/Tool/applelink.py
/usr/lib/scons/SCons/Tool/applelink.pyc
/usr/lib/scons/SCons/Tool/ar.py
/usr/lib/scons/SCons/Tool/ar.pyc
/usr/lib/scons/SCons/Tool/as.py
/usr/lib/scons/SCons/Tool/as.pyc
/usr/lib/scons/SCons/Tool/bcc32.py
/usr/lib/scons/SCons/Tool/bcc32.pyc
/usr/lib/scons/SCons/Tool/c++.py
/usr/lib/scons/SCons/Tool/c++.pyc
/usr/lib/scons/SCons/Tool/cc.py
/usr/lib/scons/SCons/Tool/cc.pyc
/usr/lib/scons/SCons/Tool/cvf.py
/usr/lib/scons/SCons/Tool/cvf.pyc
/usr/lib/scons/SCons/Tool/default.py
/usr/lib/scons/SCons/Tool/default.pyc
/usr/lib/scons/SCons/Tool/dmd.py
/usr/lib/scons/SCons/Tool/dmd.pyc
/usr/lib/scons/SCons/Tool/dvi.py
/usr/lib/scons/SCons/Tool/dvi.pyc
/usr/lib/scons/SCons/Tool/dvipdf.py
/usr/lib/scons/SCons/Tool/dvipdf.pyc
/usr/lib/scons/SCons/Tool/dvips.py
/usr/lib/scons/SCons/Tool/dvips.pyc
/usr/lib/scons/SCons/Tool/f77.py
/usr/lib/scons/SCons/Tool/f77.pyc
/usr/lib/scons/SCons/Tool/f90.py
/usr/lib/scons/SCons/Tool/f90.pyc
/usr/lib/scons/SCons/Tool/f95.py
/usr/lib/scons/SCons/Tool/f95.pyc
/usr/lib/scons/SCons/Tool/filesystem.py
/usr/lib/scons/SCons/Tool/filesystem.pyc
/usr/lib/scons/SCons/Tool/fortran.py
/usr/lib/scons/SCons/Tool/fortran.pyc
/usr/lib/scons/SCons/Tool/g++.py
/usr/lib/scons/SCons/Tool/g++.pyc
/usr/lib/scons/SCons/Tool/g77.py
/usr/lib/scons/SCons/Tool/g77.pyc
/usr/lib/scons/SCons/Tool/gas.py
/usr/lib/scons/SCons/Tool/gas.pyc
/usr/lib/scons/SCons/Tool/gcc.py
/usr/lib/scons/SCons/Tool/gcc.pyc
/usr/lib/scons/SCons/Tool/gfortran.py
/usr/lib/scons/SCons/Tool/gfortran.pyc
/usr/lib/scons/SCons/Tool/gnulink.py
/usr/lib/scons/SCons/Tool/gnulink.pyc
/usr/lib/scons/SCons/Tool/gs.py
/usr/lib/scons/SCons/Tool/gs.pyc
/usr/lib/scons/SCons/Tool/hpc++.py
/usr/lib/scons/SCons/Tool/hpc++.pyc
/usr/lib/scons/SCons/Tool/hpcc.py
/usr/lib/scons/SCons/Tool/hpcc.pyc
/usr/lib/scons/SCons/Tool/hplink.py
/usr/lib/scons/SCons/Tool/hplink.pyc
/usr/lib/scons/SCons/Tool/icc.py
/usr/lib/scons/SCons/Tool/icc.pyc
/usr/lib/scons/SCons/Tool/icl.py
/usr/lib/scons/SCons/Tool/icl.pyc
/usr/lib/scons/SCons/Tool/ifl.py
/usr/lib/scons/SCons/Tool/ifl.pyc
/usr/lib/scons/SCons/Tool/ifort.py
/usr/lib/scons/SCons/Tool/ifort.pyc
/usr/lib/scons/SCons/Tool/ilink.py
/usr/lib/scons/SCons/Tool/ilink.pyc
/usr/lib/scons/SCons/Tool/ilink32.py
/usr/lib/scons/SCons/Tool/ilink32.pyc
/usr/lib/scons/SCons/Tool/install.py
/usr/lib/scons/SCons/Tool/install.pyc
/usr/lib/scons/SCons/Tool/intelc.py
/usr/lib/scons/SCons/Tool/intelc.pyc
/usr/lib/scons/SCons/Tool/ipkg.py
/usr/lib/scons/SCons/Tool/ipkg.pyc
/usr/lib/scons/SCons/Tool/jar.py
/usr/lib/scons/SCons/Tool/jar.pyc
/usr/lib/scons/SCons/Tool/javac.py
/usr/lib/scons/SCons/Tool/javac.pyc
/usr/lib/scons/SCons/Tool/javah.py
/usr/lib/scons/SCons/Tool/javah.pyc
/usr/lib/scons/SCons/Tool/latex.py
/usr/lib/scons/SCons/Tool/latex.pyc
/usr/lib/scons/SCons/Tool/lex.py
/usr/lib/scons/SCons/Tool/lex.pyc
/usr/lib/scons/SCons/Tool/link.py
/usr/lib/scons/SCons/Tool/link.pyc
/usr/lib/scons/SCons/Tool/linkloc.py
/usr/lib/scons/SCons/Tool/linkloc.pyc
/usr/lib/scons/SCons/Tool/m4.py
/usr/lib/scons/SCons/Tool/m4.pyc
/usr/lib/scons/SCons/Tool/masm.py
/usr/lib/scons/SCons/Tool/masm.pyc
/usr/lib/scons/SCons/Tool/midl.py
/usr/lib/scons/SCons/Tool/midl.pyc
/usr/lib/scons/SCons/Tool/mingw.py
/usr/lib/scons/SCons/Tool/mingw.pyc
/usr/lib/scons/SCons/Tool/mslib.py
/usr/lib/scons/SCons/Tool/mslib.pyc
/usr/lib/scons/SCons/Tool/mslink.py
/usr/lib/scons/SCons/Tool/mslink.pyc
/usr/lib/scons/SCons/Tool/mssdk.py
/usr/lib/scons/SCons/Tool/mssdk.pyc
/usr/lib/scons/SCons/Tool/msvc.py
/usr/lib/scons/SCons/Tool/msvc.pyc
/usr/lib/scons/SCons/Tool/msvs.py
/usr/lib/scons/SCons/Tool/msvs.pyc
/usr/lib/scons/SCons/Tool/mwcc.py
/usr/lib/scons/SCons/Tool/mwcc.pyc
/usr/lib/scons/SCons/Tool/mwld.py
/usr/lib/scons/SCons/Tool/mwld.pyc
/usr/lib/scons/SCons/Tool/nasm.py
/usr/lib/scons/SCons/Tool/nasm.pyc
/usr/lib/scons/SCons/Tool/packaging/__init__.py
/usr/lib/scons/SCons/Tool/packaging/__init__.pyc
/usr/lib/scons/SCons/Tool/packaging/ipk.py
/usr/lib/scons/SCons/Tool/packaging/ipk.pyc
/usr/lib/scons/SCons/Tool/packaging/msi.py
/usr/lib/scons/SCons/Tool/packaging/msi.pyc
/usr/lib/scons/SCons/Tool/packaging/rpm.py
/usr/lib/scons/SCons/Tool/packaging/rpm.pyc
/usr/lib/scons/SCons/Tool/packaging/src_tarbz2.py
/usr/lib/scons/SCons/Tool/packaging/src_tarbz2.pyc
/usr/lib/scons/SCons/Tool/packaging/src_targz.py
/usr/lib/scons/SCons/Tool/packaging/src_targz.pyc
/usr/lib/scons/SCons/Tool/packaging/src_zip.py
/usr/lib/scons/SCons/Tool/packaging/src_zip.pyc
/usr/lib/scons/SCons/Tool/packaging/tarbz2.py
/usr/lib/scons/SCons/Tool/packaging/tarbz2.pyc
/usr/lib/scons/SCons/Tool/packaging/targz.py
/usr/lib/scons/SCons/Tool/packaging/targz.pyc
/usr/lib/scons/SCons/Tool/packaging/zip.py
/usr/lib/scons/SCons/Tool/packaging/zip.pyc
/usr/lib/scons/SCons/Tool/pdf.py
/usr/lib/scons/SCons/Tool/pdf.pyc
/usr/lib/scons/SCons/Tool/pdflatex.py
/usr/lib/scons/SCons/Tool/pdflatex.pyc
/usr/lib/scons/SCons/Tool/pdftex.py
/usr/lib/scons/SCons/Tool/pdftex.pyc
/usr/lib/scons/SCons/Tool/qt.py
/usr/lib/scons/SCons/Tool/qt.pyc
/usr/lib/scons/SCons/Tool/rmic.py
/usr/lib/scons/SCons/Tool/rmic.pyc
/usr/lib/scons/SCons/Tool/rpcgen.py
/usr/lib/scons/SCons/Tool/rpcgen.pyc
/usr/lib/scons/SCons/Tool/rpm.py
/usr/lib/scons/SCons/Tool/rpm.pyc
/usr/lib/scons/SCons/Tool/sgiar.py
/usr/lib/scons/SCons/Tool/sgiar.pyc
/usr/lib/scons/SCons/Tool/sgic++.py
/usr/lib/scons/SCons/Tool/sgic++.pyc
/usr/lib/scons/SCons/Tool/sgicc.py
/usr/lib/scons/SCons/Tool/sgicc.pyc
/usr/lib/scons/SCons/Tool/sgilink.py
/usr/lib/scons/SCons/Tool/sgilink.pyc
/usr/lib/scons/SCons/Tool/sunar.py
/usr/lib/scons/SCons/Tool/sunar.pyc
/usr/lib/scons/SCons/Tool/sunc++.py
/usr/lib/scons/SCons/Tool/sunc++.pyc
/usr/lib/scons/SCons/Tool/suncc.py
/usr/lib/scons/SCons/Tool/suncc.pyc
/usr/lib/scons/SCons/Tool/sunf77.py
/usr/lib/scons/SCons/Tool/sunf77.pyc
/usr/lib/scons/SCons/Tool/sunf90.py
/usr/lib/scons/SCons/Tool/sunf90.pyc
/usr/lib/scons/SCons/Tool/sunf95.py
/usr/lib/scons/SCons/Tool/sunf95.pyc
/usr/lib/scons/SCons/Tool/sunlink.py
/usr/lib/scons/SCons/Tool/sunlink.pyc
/usr/lib/scons/SCons/Tool/swig.py
/usr/lib/scons/SCons/Tool/swig.pyc
/usr/lib/scons/SCons/Tool/tar.py
/usr/lib/scons/SCons/Tool/tar.pyc
/usr/lib/scons/SCons/Tool/tex.py
/usr/lib/scons/SCons/Tool/tex.pyc
/usr/lib/scons/SCons/Tool/textfile.py
/usr/lib/scons/SCons/Tool/textfile.pyc
/usr/lib/scons/SCons/Tool/tlib.py
/usr/lib/scons/SCons/Tool/tlib.pyc
/usr/lib/scons/SCons/Tool/wix.py
/usr/lib/scons/SCons/Tool/wix.pyc
/usr/lib/scons/SCons/Tool/yacc.py
/usr/lib/scons/SCons/Tool/yacc.pyc
/usr/lib/scons/SCons/Tool/zip.py
/usr/lib/scons/SCons/Tool/zip.pyc
/usr/lib/scons/SCons/Util.py
/usr/lib/scons/SCons/Util.pyc
/usr/lib/scons/SCons/Variables/BoolVariable.py
/usr/lib/scons/SCons/Variables/BoolVariable.pyc
/usr/lib/scons/SCons/Variables/EnumVariable.py
/usr/lib/scons/SCons/Variables/EnumVariable.pyc
/usr/lib/scons/SCons/Variables/ListVariable.py
/usr/lib/scons/SCons/Variables/ListVariable.pyc
/usr/lib/scons/SCons/Variables/PackageVariable.py
/usr/lib/scons/SCons/Variables/PackageVariable.pyc
/usr/lib/scons/SCons/Variables/PathVariable.py
/usr/lib/scons/SCons/Variables/PathVariable.pyc
/usr/lib/scons/SCons/Variables/__init__.py
/usr/lib/scons/SCons/Variables/__init__.pyc
/usr/lib/scons/SCons/Warnings.py
/usr/lib/scons/SCons/Warnings.pyc
/usr/lib/scons/SCons/__init__.py
/usr/lib/scons/SCons/__init__.pyc
/usr/lib/scons/SCons/compat/__init__.py
/usr/lib/scons/SCons/compat/__init__.pyc
/usr/lib/scons/SCons/compat/_scons_builtins.py
/usr/lib/scons/SCons/compat/_scons_builtins.pyc
/usr/lib/scons/SCons/compat/_scons_collections.py
/usr/lib/scons/SCons/compat/_scons_collections.pyc
/usr/lib/scons/SCons/compat/_scons_dbm.py
/usr/lib/scons/SCons/compat/_scons_dbm.pyc
/usr/lib/scons/SCons/compat/_scons_hashlib.py
/usr/lib/scons/SCons/compat/_scons_hashlib.pyc
/usr/lib/scons/SCons/compat/_scons_io.py
/usr/lib/scons/SCons/compat/_scons_io.pyc
/usr/lib/scons/SCons/compat/_scons_sets.py
/usr/lib/scons/SCons/compat/_scons_sets.pyc
/usr/lib/scons/SCons/compat/_scons_subprocess.py
/usr/lib/scons/SCons/compat/_scons_subprocess.pyc
/usr/lib/scons/SCons/cpp.py
/usr/lib/scons/SCons/cpp.pyc
/usr/lib/scons/SCons/dblite.py
/usr/lib/scons/SCons/dblite.pyc
/usr/lib/scons/SCons/exitfuncs.py
/usr/lib/scons/SCons/exitfuncs.pyc
#/usr/lib/scons/scons-2.0.1.egg-info

%doc %{_mandir}/man1/scons.1*
%doc %{_mandir}/man1/sconsign.1*
%doc %{_mandir}/man1/scons-time.1*
