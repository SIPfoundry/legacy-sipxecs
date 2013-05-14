define(`sipx_packages',
@sipxecs
)

define(`repo_filename',`sipxecs.repo')
define(`repo_contents',
[sipXecs]
name=sipXecs for CentOS - \$basearch
baseurl=http://download.sipfoundry.org/pub/sipXecs/PACKAGE_VERSION()/CentOS_6/\$basearch
enabled=1
gpgcheck=0
)

dnl NOTE: You should start message with '====... if you want message to be removed after
dnl successul setup. See shell code in /root/.bashrc for details
define(`welcome_message',`
==========================
Welcome to SIPfoundry sipXecs.

After logging in as root you will automatically be taken through a setup
procedure.
')
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

ifdef(`manual-partition', 
#--- Manual Partitioning
# Let installer prompt user w/defaults.
,
#--- Auto Partitioning
zerombr yes
clearpart --all --initlabel
part /boot --fstype ext3 --size=128
part swap --size=6144
part pv.01 --size=1 --grow
volgroup vg_root pv.01
logvol / --fstype ext3 --name lv_root --vgname=vg_root --size 6144
logvol /var --fstype ext3 --name lv_var --vgname=vg_root --size=1 --grow
)

# According to anaconda docs, if you do not include a "network..." line, user is supposed
# to be prompted. I could not get this to work. so I append "asknetwork" to kernel boot
# line. UI is not as good, but it does get the job done

#--- Debugging (uncomment next line to debug in the interactive mode)
# interactive

#--- Reboot the host after installation is done
reboot

#--- Package selection
%packages
e2fsprogs
grub
kernel
gdb
strace
yum-downloadonly
nano
wget
epel-release
sipx_packages()

#--- Pre-installation script
%pre

#--- Post-installation script
%post
#!/bin/sh

#... Setup initial setup script to run one time (after initial reboot only)
cat >> /root/.bashrc <<EOF

/usr/bin/sipxecs-setup
# restore /root/.bashrc and /etc/issue to original states upon successful
# setup.
if [ \$? == 0 ]; then
  sed -i '/^\/usr\/bin\/sipxecs-setup\$/,//d' /root/.bashrc
  sed -i '/^====/,//d' /etc/issue
fi
EOF

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

# Turn off unused services that listen on ports
chkconfig portmap off
chkconfig netfs off
chkconfig nfslock off

eject
