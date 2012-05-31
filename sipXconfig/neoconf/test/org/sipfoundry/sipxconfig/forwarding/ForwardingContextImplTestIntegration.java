/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.forwarding;


import static org.junit.Assert.assertArrayEquals;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdFieldValuePresent;

import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.TimeZone;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.ScheduledDay;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.imdb.ImdbTestCase;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime.WorkingHours;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.ResultDataGrid;
import org.springframework.dao.DataAccessException;

public class ForwardingContextImplTestIntegration extends ImdbTestCase {
    private ForwardingContext m_forwardingContext;
    private CoreContext m_coreContext;
    private final Integer m_testUserId = new Integer(1000);

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }
    
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        sql("common/TestUserSeed.sql");
        sql("common/UserGroupSeed.sql");
        loadDataSet("forwarding/ScheduleSeed.xml");
        loadDataSet("forwarding/ScheduleHoursSeed.xml");
        loadDataSet("forwarding/RingSeed.xml");
        User user1000 = m_coreContext.loadUser(1000);
        getUserProfileService().saveUserProfile(user1000.getUserProfile());
        User user1001 = m_coreContext.loadUser(1001);
        getUserProfileService().saveUserProfile(user1001.getUserProfile());
        
    }

    public void testGetCallSequenceForUser() throws Exception {
        User user = m_coreContext.loadUser(m_testUserId);
        CallSequence callSequence = m_forwardingContext.getCallSequenceForUser(user);
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
        assertEquals(5, countRowsInTable("ring"));
        assertEquals(5, countRowsInTable("schedule"));
        assertEquals(9, countRowsInTable("schedule_hours"));

        m_coreContext.deleteUser(user);
        commit();
        
        assertEquals(2, countRowsInTable("ring"));
        assertEquals(3, countRowsInTable("schedule"));
        assertEquals(2, countRowsInTable("schedule_hours"));
    }

    public void testSave() throws Exception {
        User user = m_coreContext.loadUser(m_testUserId);
        CallSequence callSequence = m_forwardingContext.getCallSequenceForUser(user);
        List calls = callSequence.getRings();

        Ring ring0 = (Ring) calls.get(0);
        ring0.setType(Ring.Type.IMMEDIATE);

        Ring ring2 = (Ring) calls.get(2);
        ring2.setNumber("33");

        Ring ring1 = (Ring) calls.get(1);
        callSequence.removeRing(ring1);

        callSequence.setCfwdTime(35);

        try {
            m_forwardingContext.saveCallSequence(callSequence);
        } catch (DataAccessException e) {
            Throwable cause = e.getCause();
            System.err.println(((SQLException) cause).getNextException().getMessage());
            throw e;
        }
        Object[][] expected = new Object[][] {
                {1001, 1001, "231001", 401, true, "If no response", 0, 102},
                {1002, 1000, "33",     402, true, "If no response", 1, null},
                {1004, 1000, "231004", 404, false, "At the same time", 0, 101},
                {1005, 1001, "231005", 405, true, "If no response", 1, null},
        };
        ResultDataGrid actual = new ResultDataGrid();
        db().query("select ring_id, user_id, number, expiration, enabled, ring_type, position, " + 
                "schedule_id from ring order by ring_id", actual);
        assertArrayEquals(expected, actual.toArray());

        assertEquals(35, callSequence.getCfwdTime());
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1000", MongoConstants.CFWDTIME, 35);
    }

    public void testMove() throws Exception {
        User user = m_coreContext.loadUser(m_testUserId);
        CallSequence callSequence = m_forwardingContext.getCallSequenceForUser(user);
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

        m_forwardingContext.saveCallSequence(callSequence);

        Object[][] expected = new Object[][] {
                {1001, 1001, "231001", 401, true, "If no response", 0, 102},
                {1002, 1000, "231002", 402, true, "If no response", 0, null},
                {1003, 1000, "231003", 403, true, "If no response", 1, 100},
                {1004, 1000, "231004", 404, false, "If no response", 2, 101},
                {1005, 1001, "231005", 405, true, "If no response", 1, null}
        };
        ResultDataGrid actual = new ResultDataGrid();
        db().query("select ring_id, user_id, number, expiration, enabled, ring_type, position, " + 
                "schedule_id from ring order by ring_id", actual);
        assertArrayEquals(expected, actual.toArray());
    }

    public void testAddRing() throws Exception {
        User user = m_coreContext.loadUser(m_testUserId);
        CallSequence callSequence = m_forwardingContext.getCallSequenceForUser(user);

        Ring ring = callSequence.insertRing();
        ring.setNumber("999999");

        m_forwardingContext.saveCallSequence(callSequence);
        commit();
        Map<String, Object> actual = db().queryForMap("select * from ring where ring_id = ?", ring.getId());
        assertEquals("999999", actual.get("Number"));
        assertEquals(new Integer(3), actual.get("Position"));
    }

    public void testClearCallSequence() throws Exception {
        User user = m_coreContext.loadUser(m_testUserId);
        CallSequence callSequence = m_forwardingContext.getCallSequenceForUser(user);
        assertFalse(callSequence.getRings().isEmpty());
        int origRingCount = countRowsInTable("ring");
        int remainingRingCount = origRingCount - callSequence.getRings().size();
        m_coreContext.deleteUser(user);
        assertEquals(remainingRingCount, countRowsInTable("ring"));
    }

    public void testGetPersonalSchedulesForUserID() throws Exception {
        User user = m_coreContext.loadUser(m_testUserId);
        List<Schedule> schedules = m_forwardingContext.getPersonalSchedulesForUserId(user.getId());

        assertEquals(2, schedules.size());
        for (Schedule schedule : schedules) {
            assertEquals(user, schedule.getUser());
        }
    }

    public void testGetSchedulesByID() throws Exception {
        Schedule schedule = m_forwardingContext.getScheduleById(new Integer(100));
        Map<String, Object> actual = db().queryForMap("select * from schedule where schedule_id = ?", 100);
        assertEquals(schedule.getName(), actual.get("name"));
        assertEquals(schedule.getDescription(), actual.get("description"));
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

        m_forwardingContext.saveSchedule(schedule);
        commit();

        Map<String, Object> actual = db().queryForMap("select * from schedule where schedule_id = ?",
                schedule.getId());
        assertEquals(user.getId(), actual.get("user_id"));
        assertEquals("TestSchedule", actual.get("name"));
        assertEquals("Test Schedule", actual.get("description"));
    }

    public void testSaveDuplicateNameUserSchedule() throws Exception {
        User testUser = m_coreContext.loadUser(m_testUserId);

        Schedule userScheduleWithDuplicateName = new UserSchedule();
        userScheduleWithDuplicateName.setUser(testUser);
        userScheduleWithDuplicateName.setName("WorkingHours");
        try {
            m_forwardingContext.saveSchedule(userScheduleWithDuplicateName);
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
            m_forwardingContext.saveSchedule(userGroupScheduleWithDuplicateName);
        } catch (UserException ex) {
            assertTrue(true);
        }
    }

    public void testSaveDuplicateNameGlobalSchedule() throws Exception {
        Schedule generalScheduleWithDuplicateName = new GeneralSchedule();
        generalScheduleWithDuplicateName.setName("GeneralSchedule");
        try {
            m_forwardingContext.saveSchedule(generalScheduleWithDuplicateName);
        } catch (UserException ex) {
            assertTrue(true);
        }
    }

    public void testDeleteSchedulesById() throws Exception {
        assertEquals(5, getConnection().getRowCount("schedule"));
        List<Integer> scheduleIds = new ArrayList<Integer>();
        scheduleIds.add(new Integer(100));
        scheduleIds.add(new Integer(101));

        m_forwardingContext.deleteSchedulesById(scheduleIds);
        commit();

        assertEquals(3, countRowsInTable("schedule"));
    }

    public void testGetAllAvailableSchedulesForUser() throws Exception {
        User user = m_coreContext.loadUser(m_testUserId);
        List<Schedule> schedules = m_forwardingContext.getAllAvailableSchedulesForUser(user);

        assertEquals(2, schedules.size());
        for (Schedule schedule : schedules) {
            assertEquals(user, schedule.getUser());
        }
    }

    public void testGetAllUserGroupSchedules() throws Exception {
        List<UserGroupSchedule> allGroupSchedules = m_forwardingContext.getAllUserGroupSchedules();
        assertEquals(1, allGroupSchedules.size());
        UserGroupSchedule groupSchedule = allGroupSchedules.get(0);
        assertEquals(new Integer(103), groupSchedule.getId());
        assertEquals("MondaySchedule", groupSchedule.getName());
        assertEquals("Monday Schedule", groupSchedule.getDescription());
    }

    public void testGetAllGeneralSchedules() throws Exception {
        List<GeneralSchedule> allGeneralSchedules = m_forwardingContext.getAllGeneralSchedules();
        assertEquals(1, allGeneralSchedules.size());
        GeneralSchedule generalSchedule = allGeneralSchedules.get(0);
        assertEquals(new Integer(104), generalSchedule.getId());
        assertEquals("GeneralSchedule", generalSchedule.getName());
        assertEquals("Schedule for dialing rule", generalSchedule.getDescription());
    }

    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
