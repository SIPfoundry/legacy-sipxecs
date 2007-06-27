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

import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.TimeZone;

import org.dbunit.Assertion;
import org.dbunit.dataset.ITable;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.ScheduledDay;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.context.ApplicationContext;
import org.springframework.dao.DataAccessException;

public class ForwardingContextImplTestDb extends SipxDatabaseTestCase {
    private ForwardingContext m_context;
    private CoreContext m_coreContext;
    private Integer testUserId = new Integer(1000);

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_coreContext = (CoreContext) appContext.getBean(CoreContext.CONTEXT_BEAN_NAME);
        m_context = (ForwardingContext) appContext.getBean(ForwardingContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        TestHelper.insertFlat("admin/forwarding/ScheduleSeed.xml");
        TestHelper.insertFlat("admin/forwarding/ScheduleHoursSeed.xml");
        TestHelper.insertFlat("admin/forwarding/RingSeed.xml");
    }

    public void testGetCallSequenceForUser() throws Exception {
        User user = m_coreContext.loadUser(testUserId);
        CallSequence callSequence = m_context.getCallSequenceForUser(user);
        assertEquals(user.getId(), callSequence.getUser().getId());
        List calls = callSequence.getRings();
        assertEquals(3, calls.size());
        // test data: id, number: "23id", expiration "40(id-1000)", type - always delayed
        // ids are in reverse order staring from 1004
        int id = 1004;
        for (Iterator i = calls.iterator(); i.hasNext();) {
            Ring ring = (Ring) i.next();
            assertSame(Ring.Type.DELAYED, ring.getType());
            assertEquals("23" + ring.getId(), ring.getNumber());
            assertEquals(400 + ring.getId().intValue() - 1000, ring.getExpiration());
            assertEquals(new Integer(id--), ring.getId());
        }
    }

    public void testOnDeleteUser() throws Exception {
        User user = m_coreContext.loadUser(testUserId);
        ITable rings = TestHelper.getConnection().createDataSet().getTable("ring");
        assertEquals(5, rings.getRowCount());
        ITable schedules = TestHelper.getConnection().createDataSet().getTable("schedule");
        assertEquals(3, schedules.getRowCount());
        ITable scheduleHours = TestHelper.getConnection().createDataSet().getTable(
                "schedule_hours");
        assertEquals(8, scheduleHours.getRowCount());
        
        m_coreContext.deleteUser(user);
        
        rings = TestHelper.getConnection().createDataSet().getTable("ring");
        // 3 rings should disappear from that
        assertEquals(2, rings.getRowCount());
        schedules = TestHelper.getConnection().createDataSet().getTable("schedule");
        // 2 schedules should disappear from that
        assertEquals(1, schedules.getRowCount());
        scheduleHours = TestHelper.getConnection().createDataSet().getTable("schedule_hours");
        // 7 schedule hours should disappear from that
        assertEquals(1, scheduleHours.getRowCount());
    }

    public void testSave() throws Exception {
        User user = m_coreContext.loadUser(testUserId);
        CallSequence callSequence = m_context.getCallSequenceForUser(user);
        List calls = callSequence.getRings();

        Ring ring0 = (Ring) calls.get(0);
        ring0.setType(Ring.Type.IMMEDIATE);

        Ring ring2 = (Ring) calls.get(2);
        ring2.setNumber("33");

        Ring ring1 = (Ring) calls.get(1);
        callSequence.removeRing(ring1);

        try {
            m_context.saveCallSequence(callSequence);
            m_context.flush();
        } catch (DataAccessException e) {
            Throwable cause = e.getCause();
            System.err.println(((SQLException) cause).getNextException().getMessage());
            throw e;
        }

        ITable expected = TestHelper.loadDataSetFlat("admin/forwarding/RingModified.xml")
                .getTable("ring");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("ring");

        Assertion.assertEquals(expected, actual);
    }

    public void testMove() throws Exception {
        User user = m_coreContext.loadUser(testUserId);
        CallSequence callSequence = m_context.getCallSequenceForUser(user);
        List calls = callSequence.getRings();

        Ring ring0 = (Ring) calls.get(0);
        Ring ring2 = (Ring) calls.get(2);
        assertTrue(callSequence.moveRingDown(ring0));
        assertTrue(callSequence.moveRingDown(ring0));
        assertTrue(callSequence.moveRingUp(ring2));

        for (int i = 0; i < calls.size(); i++) {
            Ring ring = (Ring) calls.get(i);
            assertEquals(1002 + i, ring.getId().intValue());
        }

        m_context.saveCallSequence(callSequence);
        m_context.flush();

        ITable expected = TestHelper.loadDataSetFlat("admin/forwarding/RingMoved.xml").getTable(
                "ring");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("ring");
        Assertion.assertEquals(expected, actual);
    }

    public void testAddRing() throws Exception {
        User user = m_coreContext.loadUser(testUserId);
        CallSequence callSequence = m_context.getCallSequenceForUser(user);

        Ring ring = callSequence.insertRing();
        ring.setNumber("999999");

        m_context.saveCallSequence(callSequence);
        m_context.flush();

        ITable actual = TestHelper.getConnection().createDataSet().getTable("ring");
        assertEquals("999999", actual.getValue(0, "Number"));
        assertEquals(new Integer(3), actual.getValue(0, "Position"));
    }

    public void testGetForwardingAliasesAndAuthExceptions() throws Exception {

        // FIXME insert real permissions
        // TestHelper.update("admin/forwarding/permissions.xml");

        // this just tests that all aliases and excpetions are processed by the context
        // there are separate test that take care of the content testing
        // one of the rings is disabled and should not count towards the number
        // of generated aliases
        int seedRings = 4;
        Collection forwardingAliases = m_context.getAliasMappings();
        assertEquals(seedRings, forwardingAliases.size());
        List authExceptions = m_context.getForwardingAuthExceptions();

        // FIXME, should be 3
        assertEquals(0, authExceptions.size());

        User user = m_coreContext.loadUser(testUserId);
        CallSequence callSequence = m_context.getCallSequenceForUser(user);
        Ring ring = callSequence.insertRing();
        ring.setNumber("999999");

        m_context.saveCallSequence(callSequence);
        m_context.flush();

        forwardingAliases = m_context.getAliasMappings();
        assertEquals(seedRings + 1, forwardingAliases.size());
        authExceptions = m_context.getForwardingAuthExceptions();

        // FIXME, should be 4
        assertEquals(0, authExceptions.size());
    }

    public void testClearCallSequence() throws Exception {
        User user = m_coreContext.loadUser(testUserId);
        CallSequence callSequence = m_context.getCallSequenceForUser(user);

        ITable ringTable = TestHelper.getConnection().createDataSet().getTable("ring");
        assertFalse(callSequence.getRings().isEmpty());

        int remainingRingCount = ringTable.getRowCount() - callSequence.getRings().size();

        m_context.removeCallSequenceForUserId(user.getId());

        ringTable = TestHelper.getConnection().createDataSet().getTable("ring");
        assertEquals(remainingRingCount, ringTable.getRowCount());
    }

    public void testGetSchedulesForUserID() throws Exception {
        User user = m_coreContext.loadUser(testUserId);
        List<Schedule> schedules = m_context.getSchedulesForUserId(user.getId());

        assertEquals(2, schedules.size());
        for (Schedule schedule : schedules) {
            assertEquals(user, schedule.getUser());
        }
    }

    public void testGetSchedulesByID() throws Exception {
        Schedule schedule = m_context.getScheduleById(new Integer(100));

        ITable actual = TestHelper.getConnection().createDataSet().getTable("schedule");
        assertEquals(schedule.getName(), actual.getValue(0, "name"));
        assertEquals(schedule.getDescription(), actual.getValue(0, "description"));
    }

    public void testSaveSchedule() throws Exception {
        User user = m_coreContext.loadUser(testUserId);

        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        c.set(1970, Calendar.JANUARY, 1);
        WorkingTime wt = new WorkingTime();

        WorkingHours[] hours = new WorkingHours[1];
        hours[0] = new WorkingHours();
        hours[0].setDay(ScheduledDay.SUNDAY);
        c.set(Calendar.HOUR_OF_DAY, 9);
        c.set(Calendar.MINUTE, 0);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 18);
        c.set(Calendar.MINUTE, 0);
        hours[0].setStop(c.getTime());

        wt.setWorkingHours(hours);

        Schedule schedule = new Schedule();
        schedule.setUser(user);
        schedule.setName("TestSchedule");
        schedule.setDescription("Test Schedule");

        m_context.saveSchedule(schedule);
        m_context.flush();

        ITable actualSchedules = TestHelper.getConnection().createDataSet().getTable("schedule");
        assertEquals(user.getId(), actualSchedules.getValue(0, "user_id"));
        assertEquals("TestSchedule", actualSchedules.getValue(0, "name"));
        assertEquals("Test Schedule", actualSchedules.getValue(0, "description"));
    }

    public void testDeleteSchedulesById() throws Exception {
        List<Integer> scheduleIds = new ArrayList<Integer>();
        scheduleIds.add(new Integer(100));
        scheduleIds.add(new Integer(101));

        List<Schedule> schedulesWithRings = m_context.deleteSchedulesById(scheduleIds);
        assertEquals(2, schedulesWithRings.size());
    }
}
