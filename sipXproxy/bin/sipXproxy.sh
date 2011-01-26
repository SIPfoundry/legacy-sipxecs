#!/bin/sh
#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

Action=RUN
Status=0
Args=""

while [ $# -ne 0 ]
do
    case ${1} in
        --configtest)
            Action=CONFIGTEST
            ;;

        --stop)
            Action=STOP
            ;;

        *)
            Args="$Args $1"
            ;;
    esac           

    shift # always consume 1
done

. /home/joegen/sipxecs-bin/libexec/sipXecs/sipx-utils.sh || exit 1

pidfile="/home/joegen/sipxecs-bin/var/run/sipxpbx/sipXproxy.pid"

case ${Action} in
   RUN)
     echo $$ > ${pidfile}
     exec /home/joegen/sipxecs-bin/bin/sipXproxy $Args
     ;;

   STOP)
     sipx_stop sipXproxy ${pidfile}
     ;;

   CONFIGTEST)
     myDomain=`sipx_config_value /home/joegen/sipxecs-bin/etc/sipxpbx/domain-config SIP_DOMAIN_NAME`
     Status=$((${Status} + $?))
     sipx_config_exists /home/joegen/sipxecs-bin/etc/sipxpbx/sipXproxy-config
     proxy_config_status=$?
     Status=$((${Status} + ${proxy_config_status}))

     # check validity of xml routing rules, and authorization rules
     /home/joegen/sipxecs-bin/bin/sipx-validate-xml /home/joegen/sipxecs-bin/etc/sipxpbx/forwardingrules.xml
     Status=$((${Status} + $?))
     /home/joegen/sipxecs-bin/bin/sipx-validate-xml /home/joegen/sipxecs-bin/etc/sipxpbx/authrules.xml
     Status=$((${Status} + $?))
     /home/joegen/sipxecs-bin/bin/sipx-validate-xml /home/joegen/sipxecs-bin/etc/sipxpbx/nattraversalrules.xml
     Status=$((${Status} + $?))

     # Check that the log file is writable.
     logfile="/home/joegen/sipxecs-bin/var/log/sipxpbx/sipXproxy.log"
     if [ -e $logfile -a ! -w $logfile ]
     then
         echo "Log file '$logfile' exists but is not writable by user 'joegen'." >&2
         Status=1
     fi

     if [ ${proxy_config_status} -eq 0 ]
     then
         myIp=`sipx_config_value /home/joegen/sipxecs-bin/etc/sipxpbx/sipXproxy-config SIPX_PROXY_BIND_IP 2> /dev/null`
         if ! sip_resolves_to ${myDomain} ${myIp}
         then
             cat <<ERROR
    SIP route to SIPXCHANGE_DOMAIN_NAME '${myDomain}' is not to my IP address: ${myIp}
    See the installation guide on setting the DNS SRV records and domain names
ERROR
             Status=1
         fi
     fi
     ;;
esac

exit $Status

