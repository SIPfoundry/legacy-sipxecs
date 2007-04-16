Summary: Unrestricted security policy files
Name: java
Version: ibm
Release: unrestricted
License: Non-distributable, restricted use
Group: Security/Cryptography
URL: http://www.ibm.com
Source0: unrestrict142.zip
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%define origin          ibm
%define javaver         1.5.0
%define jrelnk          jre-%{javaver}-%{origin}

# Sepculate that one higher that main RPM makes alternatives take this policy ???
%define priority        1503
%define JreSecurityDir  %{_jvmprivdir}/java-%{javaver}-%{origin}/jce/unrestrict

Requires:	/usr/sbin/update-alternatives
Requires:       java-%{javaver}-%{origin}

%description

%prep

%build

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{JreSecurityDir}
unzip -o -d $RPM_BUILD_ROOT%{JreSecurityDir} $RPM_SOURCE_DIR/unrestrict142.zip

%clean
rm -rf $RPM_BUILD_ROOT

%post
if [ -d %{_jvmdir}/%{jrelnk}/lib/security ]; then
  # Need to remove the old jars in order to support upgrading, ugly :(
  # update-alternatives fails silently if the link targets exist as files.
  rm -f %{_jvmdir}/%{jrelnk}/lib/security/{local,US_export}_policy.jar
fi
/usr/sbin/update-alternatives --install \
    %{_jvmdir}/%{jrelnk}/lib/security/local_policy.jar \
   jce_%{javaver}_%{origin}_local_policy \
   %{JreSecurityDir}/local_policy.jar \
   %{priority} \
  --slave \
    %{_jvmdir}/%{jrelnk}/lib/security/US_export_policy.jar \
   jce_%{javaver}_%{origin}_us_export_policy \
   %{JreSecurityDir}/US_export_policy.jar

%postun
update-alternatives --remove \
  jce_%{javaver}_%{origin}_unrestricted_local_policy  \
  %{JreSecurityDir}/local_policy.jar

%files
%defattr(-,root,root,-)
%{JreSecurityDir}/local_policy.jar
%{JreSecurityDir}/US_export_policy.jar

%doc

%changelog
* Wed Mar  8 2006 Douglas Hubler <dhubler@pingtel.com> - ibm-unrestricted
- Initial build.

