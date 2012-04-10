# TODO
# - individtual php-pear-XXX packages
#   http://phpseclib.sourceforge.net/pear.htm
#   pear remote-list -c phpseclib
#
# SPEC and patch from
#  http://cvs.pld-linux.org/cgi-bin/viewvc.cgi/cvs/packages/php-seclib
#
%define     subver     a
%define		rel		1
%define		pkgname	seclib
%define		php_min_version 5.2.0
%include	/usr/lib/rpm/macros.php
Summary:	PHP Secure Communications Library
Name:		php-%{pkgname}
Version:	0.2.1
Release:	0.%{subver}.%{rel}
License:	LGPL
Group:		Development/Languages/PHP
Source0:	phpseclib%{version}%{subver}.zip
# Source0-md5:	028be0414123f4bff61f1b2b4fd68fab
URL:		http://phpseclib.sourceforge.net/

# Pulled from PLD's rpm-build-macros --Douglas
%define __find /usr/bin/find
%define undos(f:i)     %{-f:%{__find} -regextype posix-extended -%{-i:i}regex '^.+\\.(%(echo %{-f*} | tr ',' '|'))$' -print0 | xargs -0} %{__sed} -i -e 's,\\r$,,' %* \
%{nil}

BuildRequires:	php-pear

BuildRequires:	unzip
Patch0:		includes.patch
Requires:	php-bcmath
Requires:	php-common >= 4:%{php_min_version}
Requires:	php-date
Requires:	php-hash
Requires:	php-openssl
Requires:	php-pcre
Requires:	php-pear
Requires:	php-xml
BuildArch:	noarch
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)

%description
Pure-PHP implementations of an arbitrary-precision integer arithmetic
library, fully PKCS#1 (v2.1) compliant RSA, DES, 3DES, RC4, Rijndael,
AES, SSH-1, SSH-2, and SFTP.

%prep
%setup -qc
%undos -f php,html,css
%patch0 -p1

mkdir html
mv *.html *.css html

# php 4.2, php 5.0
rm PHP/Compat/Function/array_fill.php
# php 5.0
rm PHP/Compat/Function/bcpowmod.php
# php 5.0
rm PHP/Compat/Function/str_split.php

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT%{php_pear_dir}
cp -a PHP Crypt Math Net $RPM_BUILD_ROOT%{php_pear_dir}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%doc html
%{php_pear_dir}/Crypt/AES.php
%{php_pear_dir}/Crypt/DES.php
%{php_pear_dir}/Crypt/Hash.php
%{php_pear_dir}/Crypt/RC4.php
%{php_pear_dir}/Crypt/RSA.php
%{php_pear_dir}/Crypt/Random.php
%{php_pear_dir}/Crypt/Rijndael.php
%{php_pear_dir}/Crypt/TripleDES.php
%{php_pear_dir}/Math/BigInteger.php
%{php_pear_dir}/Net/SFTP.php
%{php_pear_dir}/Net/SSH1.php
%{php_pear_dir}/Net/SSH2.php

%changelog
* Thu Mar 03 2012 Douglas Hubler <dhubler@ezuce.com>
- Build on CentOS 6 w/o other PLD packages
- OLD NOTES:
- Revision 1.4  2011/01/23 23:14:53  sparky
- BR: unzip
- Revision 1.3  2010/08/31 06:57:18  glen
- release 1
- Revision 1.2  2010/08/31 06:57:05  glen
- fix include paths
- Revision 1.1  2010/08/31 06:54:17  glen
- initial, bundle all phpseclib packages for now
