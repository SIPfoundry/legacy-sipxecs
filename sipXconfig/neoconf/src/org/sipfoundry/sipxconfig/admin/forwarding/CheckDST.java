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
import java.util.Date;
import java.util.TimeZone;
import java.util.Timer;
import java.util.TimerTask;

/**
 * Time task that runs once a day and checks if DST will change during next 24 hours. If DST
 * change is going to happen, it schedules notification that regenerates aliases.
 */
public class CheckDST extends TimerTask {

    private ForwardingContext m_fwdContext;

    public void setFwdContext(ForwardingContext context) {
        m_fwdContext = context;
    }

    private void setupNotifyTask(Date dstChangeTime) {
        Timer timer = new Timer();

        timer.schedule(new TimerTask() {
            public void run() {
                m_fwdContext.notifyCommserver();
            }
        }, dstChangeTime);
    }

    public void run() {
        TimeZone tzLocal = TimeZone.getDefault();
        Date dstChangeTime = findDstChangeTime(tzLocal, new Date());
        if (dstChangeTime != null) {
            setupNotifyTask(dstChangeTime);
        }
    }

    /**
     * Find the aproximate time of DTS change
     * 
     * @param tz time zone
     * @param today time from which we will start checking
     * @return null if not DST change in next 24 hours, otherwise time of the spproximate DST
     *         switch (after switch happens)
     */
    Date findDstChangeTime(TimeZone tz, Date today) {
        Calendar calendar = Calendar.getInstance(tz);
        calendar.setTime(today);

        // check one day ahead
        calendar.roll(Calendar.DAY_OF_MONTH, true);
        Date future = calendar.getTime();
        boolean dtsNow = tz.inDaylightTime(today);

        Date dstChangeTime = null;

        // find when it changes
        while (dtsNow != tz.inDaylightTime(future)) {
            dstChangeTime = future;
            calendar.roll(Calendar.HOUR_OF_DAY, false);
            future = calendar.getTime();
        }
        return dstChangeTime;
    }
}
