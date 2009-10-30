#!/bin/sh
#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

Action=RUN
Status=0
Args=""

# Check if the process is alive
proc_alive() # pid
{
   2>/dev/null kill -0 $1
   return $?
}

sipx_stop() {
   proc_name=$1; shift
   pidFile=$1; shift

   STATUS=0

   echo -n "  Confirm Stop: $proc_name "

   if [ -r ${pidFile} ]
   then
       PID=`cat ${pidFile} 2> /dev/null`
       if proc_alive $PID
       then
          kill $PID 2> /dev/null
          for ticks in `seq 19 -1 0`
          do
             sleep 1
             proc_alive "$PID" && echo -n "." || break
          done
          if proc_alive "$PID"
          then
              # It didn't die!  Try to get a core dump
              echo ""
              echo    "    $proc_name did not Stop."
              echo -n "    Trying to abort $proc_name "
              kill -ABRT $PID 2> /dev/null
              for ticks in `seq 55 -5 0`
              do
                 sleep 5
                 proc_alive "$PID" && echo -n " $ticks" || break
              done
              if proc_alive "$PID"
              then
                  echo ""
                  echo    "    $proc_name did not abort."
                  echo -n "    Trying to kill $proc_name "
                  kill -KILL $PID 2> /dev/null
                  for ticks in `seq 55 -5 0`
                  do
                     sleep 5
                     proc_alive "$PID" && echo -n " $ticks" || break
                  done
              fi
          fi
       fi

       if proc_alive "$PID"
       then
           # Holy fluff!  It didn't die!  The end is nigh!
           echo ""
           echo_failure
           echo ""
           [ "$BOOTUP" = "color" ] && $SETCOLOR_FAILURE
           echo "    $proc_name will not die."
           echo "    Machine must be restarted!"
           [ "$BOOTUP" = "color" ] && $SETCOLOR_NORMAL
       else
           echo echo_success
           echo ""
           rm -f ${pidFile}
       fi

   else
       echo -n "(Not started) "
       echo echo_success
       echo ""
   fi
}

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
            Args="$1"
            ;;
    esac

    shift # always consume 1
done

pidfile=/tmp/$Args.pid

case ${Action} in
   RUN)
     echo $$ > ${pidfile}
     echo "faking start for $Args"
     for ticks in `seq 49 -1 0`
     do
        sleep 1;
     done;
     ;;

   STOP)
     echo killing $Args...
     sipx_stop $Args ${pidfile}
     ;;

   CONFIGTEST)
     Status=0
     echo faking configtest for $Args...
     sleep 0.5
     ;;
esac

exit $Status
