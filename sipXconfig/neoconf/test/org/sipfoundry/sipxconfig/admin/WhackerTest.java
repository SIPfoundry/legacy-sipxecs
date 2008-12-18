/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.Calendar;
import java.util.Collections;
import java.util.List;

import junit.framework.AssertionFailedError;
import junit.framework.TestCase;

import org.easymock.classextension.EasyMock;
import org.easymock.classextension.IMocksControl;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.service.SipxMediaService;
import org.sipfoundry.sipxconfig.service.SipxService;

public class WhackerTest extends TestCase {

    public void testWhacker() throws Exception {
        SipxService sipxMediaService = new SipxMediaService();
        List<SipxService> services = Collections.singletonList(sipxMediaService);

        Whacker whacker = new Whacker();
        whacker.setServices(services);

        // The Whacker is supposed to do a restart through the processContext.
        // Make a mock control that checks that.
        IMocksControl m_processControl = EasyMock.createStrictControl();
        SipxProcessContext processContext = m_processControl.createMock(SipxProcessContext.class);
        processContext.manageServices(services, SipxProcessContext.Command.RESTART);
        m_processControl.replay();

        whacker.setProcessContext(processContext);

        // Set the WhackerTask to run right away so we don't get bored waiting for it.
        // Use the allowStaleDate test hack, otherwise the date will land in the past
        // and get pushed to the future.
        Calendar date = Calendar.getInstance();
        whacker.setHours(date.get(Calendar.HOUR_OF_DAY));
        whacker.setMinutes(date.get(Calendar.MINUTE));
        whacker.setAllowStaleDate(true);

        // Run the Whacker, simulating app startup
        whacker.setEnabled(true); // in case it is disabled via properties file
        whacker.setScheduledDay(ScheduledDay.EVERYDAY.getName());
        whacker.onApplicationEvent(new ApplicationInitializedEvent(this));

        // Wait for a second to make sure the task has run, then verify
        Thread.sleep(1000);
        try {
            m_processControl.verify();
        } catch (AssertionFailedError e) {
            fail("processContext.manageServices was not called by the TimerTask.  Be aware that this test is timing-dependent, you may need to tweak the TimerTask time or the sleep time for the main thread.");
        }
    }

    public void testGetScheduledDay() {
        Whacker whacker = new Whacker();

        String dayNames[] = new String[] {
            "Every day", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
        };
        ScheduledDay[] days = new ScheduledDay[] {
            ScheduledDay.EVERYDAY, ScheduledDay.SUNDAY, ScheduledDay.MONDAY, ScheduledDay.TUESDAY,
            ScheduledDay.WEDNESDAY, ScheduledDay.THURSDAY, ScheduledDay.FRIDAY, ScheduledDay.SATURDAY
        };
        for (int i = 0; i < days.length; i++) {
            whacker.setScheduledDay(dayNames[i]);
            assertTrue(whacker.getScheduledDayEnum() == days[i]);
        }
    }

}
