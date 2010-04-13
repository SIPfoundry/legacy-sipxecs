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
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.context.ApplicationContext;
import org.springframework.dao.DataAccessException;

public class ForwardingContextImplTestDb extends SipxDatabaseTestCase {
    private ForwardingContext m_context;
    private CoreContext m_coreContext;
    private final Integer m_testUserId = new Integer(1000);

    @Override
    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_coreContext = (CoreContext) appContext.getBean(CoreContext.CONTEXT_BEAN_NAME);
        m_context = (ForwardingContext) appContext.getBean(ForwardingContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        TestHelper.insertFlat("common/UserGroupSeed.db.xml");
        TestHelper.insertFlat("admin/forwarding/ScheduleSeed.xml");
        TestHelper.insertFlat("admin/forwarding/ScheduleHoursSeed.xml");
        TestHelper.insertFlat("admin/forwarding/RingSeed.xml");
    }

    public void testGetCallSequenceForUser() throws Exception {
        User user = m_coreContext.loadUser(m_testUserId);
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
        User user = m_coreContext.loadUser(m_testUserId);
        ITable rings = TestHelper.getConnection().createDataSet().getTable("ring");
        assertEquals(5, rings.getRowCount());
        ITable schedules = TestHelper.getConnection().createDataSet().getTable("schedule");
        assertEquals(5, schedules.getRowCount());
        ITable scheduleHours = TestHelper.getConnection().createDataSet().getTable(
                "schedule_hours");
        assertEquals(9, scheduleHours.getRowCount());

        m_coreContext.deleteUser(user);

        rings = TestHelper.getConnection().createDataSet().getTable("ring");
        // 3 rings should disappear from that
        assertEquals(2, rings.getRowCount());
        schedules = TestHelper.getConnection().createDataSet().getTable("schedule");
        // 2 schedules should disappear from that
        assertEquals(3, schedules.getRowCount());
        scheduleHours = TestHelper.getConnection().createDataSet().getTable("schedule_hours");
        // 7 schedule hours should disappear from that
        assertEquals(2, scheduleHours.getRowCount());
    }

    public void testSave() throws Exception {
        User user = m_coreContext.loadUser(m_testUserId);
        CallSequence callSequence = m_context.getCallSequenceForUser(user);
        List calls = callSequence.getRings();

        Ring ring0 = (Ring) calls.get(0);
        ring0.setType(Ring.Type.IMMEDIATE);

        Ring ring2 = (Ring) calls.get(2);
        ring2.setNumber("33");

        Ring ring1 = (Ring) calls.get(1);
        callSequence.removeRing(ring1);

        callSequence.setCfwdTime(35);

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

        assertEquals(35, callSequence.getCfwdTime());
    }

    public void testMove() throws Exception {
        User user = m_coreContext.loadUser(m_testUserId);
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
        User user = m_coreContext.loadUser(m_testUserId);
        CallSequence callSequence = m_context.getCallSequenceForUser(user);

        Ring ring = callSequence.insertRing();
        ring.setNumber("999999");

        m_context.saveCallSequence(callSequence);
        m_context.flush();

        ITable actual = TestHelper.getConnection().createDataSet().getTable("ring");
        assertEquals("999999", actual.getValue(0, "Number"));
        assertEquals(new Integer(3), actual.getValue(0, "Position"));
    }

    public void testClearCallSequence() throws Exception {
        User user = m_coreContext.loadUser(m_testUserId);
        CallSequence callSequence = m_context.getCallSequenceForUser(user);

        ITable ringTable = TestHelper.getConnection().createDataSet().getTable("ring");
        assertFalse(callSequence.getRings().isEmpty());

        int remainingRingCount = ringTable.getRowCount() - callSequence.getRings().size();

        m_context.removeCallSequenceForUserId(user.getId());

        ringTable = TestHelper.getConnection().createDataSet().getTable("ring");
        assertEquals(remainingRingCount, ringTable.getRowCount());
    }

    public void testGetPersonalSchedulesForUserID() throws Exception {
        User user = m_coreContext.loadUser(m_testUserId);
        List<Schedule> schedules = m_context.getPersonalSchedulesForUserId(user.getId());

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
        User user = m_coreContext.loadUser(m_testUserId);

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

        Schedule schedule = new UserSchedule();
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

    public void testSaveDuplicateNameUserSchedule() throws Exception {
        User testUser = m_coreContext.loadUser(m_testUserId);

        Schedule userScheduleWithDuplicateName = new UserSchedule();
        userScheduleWithDuplicateName.setUser(testUser);
        userScheduleWithDuplicateName.setName("WorkingHours");
        try {
            m_context.saveSchedule(userScheduleWithDuplicateName);
        } catch (UserException ex) {
            assertTrue(true);
        }
    }

    public void testSaveDuplicateNameUserGroupSchedule() throws Exception {
        User user = m_coreContext.loadUser(new Integer(1001));
        Schedule userGroupScheduleWithDuplicateName = new UserGroupSchedule();
        userGroupScheduleWithDuplicateName.setUser(user);
        userGroupScheduleWithDuplicateName.setUserGroup(user.getGroupsAsList().get(0));
        userGroupScheduleWithDuplicateName.setName("MondaySchedule");
        try {
            m_context.saveSchedule(userGroupScheduleWithDuplicateName);
        } catch (UserException ex) {
            assertTrue(true);
        }
    }

    public void testSaveDuplicateNameGlobalSchedule() throws Exception {
        Schedule generalScheduleWithDuplicateName = new GeneralSchedule();
        generalScheduleWithDuplicateName.setName("GeneralSchedule");
        try {
            m_context.saveSchedule(generalScheduleWithDuplicateName);
        } catch (UserException ex) {
            assertTrue(true);
        }
    }

    public void testDeleteSchedulesById() throws Exception {
        assertEquals(5, getConnection().getRowCount("schedule"));
        List<Integer> scheduleIds = new ArrayList<Integer>();
        scheduleIds.add(new Integer(100));
        scheduleIds.add(new Integer(101));

        m_context.deleteSchedulesById(scheduleIds);

        assertEquals(3, getConnection().getRowCount("schedule"));
    }

    public void testGetAllAvailableSchedulesForUser() throws Exception {
        User user = m_coreContext.loadUser(m_testUserId);
        List<Schedule> schedules = m_context.getAllAvailableSchedulesForUser(user);

        assertEquals(2, schedules.size());
        for (Schedule schedule : schedules) {
            assertEquals(user, schedule.getUser());
        }
    }

    public void testGetAllUserGroupSchedules() throws Exception {
        List<UserGroupSchedule> allGroupSchedules = m_context.getAllUserGroupSchedules();
        assertEquals(1, allGroupSchedules.size());
        UserGroupSchedule groupSchedule = allGroupSchedules.get(0);
        assertEquals(new Integer(103), groupSchedule.getId());
        assertEquals("MondaySchedule", groupSchedule.getName());
        assertEquals("Monday Schedule", groupSchedule.getDescription());
    }

    public void testGetAllGeneralSchedules() throws Exception {
        List<GeneralSchedule> allGeneralSchedules = m_context.getAllGeneralSchedules();
        assertEquals(1, allGeneralSchedules.size());
        GeneralSchedule generalSchedule = allGeneralSchedules.get(0);
        assertEquals(new Integer(104), generalSchedule.getId());
        assertEquals("GeneralSchedule", generalSchedule.getName());
        assertEquals("Schedule for dialing rule", generalSchedule.getDescription());
    }
}
