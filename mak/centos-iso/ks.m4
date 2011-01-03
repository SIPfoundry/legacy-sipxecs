define(`sipx_packages', 
e2fsprogs
grub
kernel
ntp
dhcp
bind
caching-nameserver
gdb
strace
yum-downloadonly
sipxecs
)

define(`repo_filename',`sipxecs.repo')
define(`repo_contents',
[sipXecs]
name=sipXecs build service for CentOS 5 - \$basearch
baseurl=http://download.ezuce.com/sipfoundry/PACKAGE_VERSION()/CentOS_5/$basearch
enabled=1
gpgcheck=0
)

define(`welcome_message',
 
Welcome to SIPfoundry sipXecs
=============================
First time logon: user = root     password = setup

)
sinclude(`oem.m4')
##################################################
###
### Kickstart configuration file
### for sipXecs distribution CDs
###
##################################################

#--- Installation method (install, no upgrade) and source (CD-ROM)
install
cdrom

#--- Debugging (uncomment next line to debug in the interactive mode)
#interactive

#--- Language and input support
lang en_US.UTF-8
langsupport --default=en_US.UTF-8 en_US.UTF-8
keyboard us
mouse generic3ps/2

#--- X-Windows (use "skipx" directive to skip X-Windows configuration)
skipx

#--- Network configuration
# Add some default or else Anaconda will pop a window and ask
network --device eth0 --bootproto dhcp

#--- Authentication and security
rootpw setup
firewall --disabled
selinux --disabled
authconfig --enableshadow --enablemd5

#--- Time zone
timezone America/New_York

#--- Boot loader
bootloader --location=mbr

#--- Partitioning
ifdef(`manual-partition', ,
zerombr yes
clearpart --all --initlabel
part /boot --fstype ext3 --size=128
part swap --size=6144
part / --fstype ext3 --size=6144
part /var --fstype ext3 --size 1 --grow
)

#--- Reboot the host after installation is done
reboot

#--- Package selection
%packages --resolvedeps
sipx_packages()

#--- Pre-installation script
%pre

#--- Post-installation script
%post
#!/bin/sh

#... Setup initial setup script to run one time (after initial reboot only)
echo -e "\n/usr/bin/sipxecs-setup-system\n" >> /root/.bashrc
# the script removes itself from the root .bashrc file when it completes

#... Add logon message
cat >> /etc/issue <<EOF
welcome_message()
EOF

cat > /etc/yum.repos.d/repo_filename() <<EOF
repo_contents()
EOF

#... Boot kernel in quiet mode
sed -i 's/ro root/ro quiet root/g' /boot/grub/grub.conf

#... Prevent sipxecs from starting after first reboot as it is not yet configured
chkconfig sipxecs off

# Postgresql will be started by sipxecs
chkconfig postgresql off

# Turn off unused services that listen on ports
chkconfig portmap off
chkconfig netfs off
chkconfig nfslock off

eject
