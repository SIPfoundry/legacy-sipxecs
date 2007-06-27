/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.forwarding;

import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.TimeZone;
import java.util.Timer;
import java.util.TimerTask;

public class CheckDST extends TimerTask {

    private ForwardingContext m_fwdContext;

    public CheckDST() {
    }

    public ForwardingContext getFwdContext() {
        return m_fwdContext;
    }

    public void setFwdContext(ForwardingContext context) {
        this.m_fwdContext = context;
    }

    public void run() {
        TimeZone tzLocal = TimeZone.getDefault();

        // today
        GregorianCalendar today = new GregorianCalendar();
        today.set(Calendar.DATE, today.get(Calendar.DATE));
        // tomorrow
        GregorianCalendar tomorrow = new GregorianCalendar();
        tomorrow.set(Calendar.DATE, tomorrow.get(Calendar.DATE) + 1);

        // if tomorrow the DST state will change
        if (tzLocal.inDaylightTime(today.getTime()) != tzLocal.inDaylightTime(tomorrow.getTime())) {
            GregorianCalendar dstChangeCalendar = null;
            // try from hour to hour to see when it will happen
            for (int i = 0; i < 24; i++) {
                today.set(Calendar.HOUR, today.get(Calendar.HOUR) + 1);
                if (tzLocal.inDaylightTime(tomorrow.getTime()) == tzLocal.inDaylightTime(today
                        .getTime())) {
                    dstChangeCalendar = new GregorianCalendar();
                    dstChangeCalendar.set(Calendar.DATE, today.get(Calendar.DATE));
                    dstChangeCalendar.set(Calendar.HOUR_OF_DAY, today.get(Calendar.HOUR_OF_DAY));
                    dstChangeCalendar.set(Calendar.MINUTE, 0);
                    dstChangeCalendar.set(Calendar.SECOND, 0);
                    // break if we found the hour
                    break;
                }
            }

            // create a timer task that generates the alias at DST change hour
            if (dstChangeCalendar != null) {
                Timer timer = new Timer();

                timer.schedule(new TimerTask() {
                    public void run() {
                        getFwdContext().notifyCommserver();
                    }
                }, dstChangeCalendar.getTime());
            }

        }

    }
}
