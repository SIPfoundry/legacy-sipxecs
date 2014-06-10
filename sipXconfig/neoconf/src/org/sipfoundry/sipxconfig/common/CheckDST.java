/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;
import java.util.Timer;
import java.util.TimerTask;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

/**
 * Time task that runs once a day and checks if DST will change during next 24 hours. If DST
 * change is going to happen, it schedules notification that regenerates aliases.
 */
public class CheckDST implements ApplicationContextAware {
    private static final Log LOG = LogFactory.getLog(CheckDST.class);

    private ApplicationContext m_applicationContext;

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }

    private void setupNotifyTask(Date dstChangeTime) {
        Timer timer = new Timer();

        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                LOG.info("Triggering DST event...");
                m_applicationContext.publishEvent(new DSTChangeEvent(this));
            }
        }, dstChangeTime);
    }

    public void checkDst() {
        LOG.info("Checking DST...");
        TimeZone tzLocal = TimeZone.getDefault();
        Date dstChangeTime = findDstChangeTime(tzLocal, new Date());
        if (dstChangeTime != null) {
            LOG.info("DST change detected at " + dstChangeTime.toString());
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
            if (calendar.get(Calendar.HOUR_OF_DAY) == 23) {
                calendar.roll(Calendar.DAY_OF_MONTH, false);
            }
            future = calendar.getTime();
        }
        return dstChangeTime;
    }
}
