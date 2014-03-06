/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.dialplan;

import static org.sipfoundry.commons.mongo.MongoConstants.ALIASES;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;

import org.sipfoundry.sipxconfig.common.ScheduledDay;
import org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime.WorkingHours;
import org.sipfoundry.sipxconfig.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.forwarding.GeneralSchedule;
import org.sipfoundry.sipxconfig.forwarding.Schedule;
import org.sipfoundry.sipxconfig.test.MongoTestIntegration;
import org.springframework.beans.factory.annotation.Required;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

public class AttendantRuleTestIntegration extends MongoTestIntegration {

    private DialPlanContext m_dialPlanContext;
    private DialPlanSetup m_dialPlanSetup;
    private ForwardingContext m_forwardingContext;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        getEntityCollection().drop();
    }

    public void testLiveAttendantRule() throws Exception {
        m_dialPlanSetup.setupDefaultRegion();

        // disabled attendant rule
        AttendantRule rule = new AttendantRule();
        rule.setName("liveAttendant");
        rule.setExtension("160");
        rule.setAttendantAliases("6 live");
        rule.setDid("+123456789");
        rule.setEnabled(false);
        m_dialPlanContext.storeRule(rule);
        List<DialingRule> rules = m_dialPlanContext.getRules();
        assertTrue(rules.contains(rule));
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            "ent"
        }, new String[] {
            "attendantrule"
        });

        // enable rule, live attendant disabled by default
        rule.setEnabled(true);
        m_dialPlanContext.storeRule(rule);
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            "ent"
        }, new String[] {
            "attendantrule"
        });

        // enable live attendant
        rule.setLiveAttendant(true);
        rule.setLiveAttendantExtension("201");
        rule.setLiveAttendantRingFor(10);
        rule.setFollowUserCallForward(false);
        m_dialPlanContext.storeRule(rule);
        MongoTestCaseHelper.assertObjectWithFieldsValuesPresent(getEntityCollection(), new String[] {
            "ent", "ident", "cnt"
        }, new String[] {
            "attendantrule", "160@example.org", "sip:160@example.org"
        });
        List<DBObject> aliasesList = new ArrayList<DBObject>();
        DBObject als = new BasicDBObject();
        als.put("id", "160");
        als.put("cnt", "<sip:201@example.org;sipx-noroute=Voicemail;sipx-userforward=false?expires=10>;q=0.933");
        als.put("rln", "userforward");
        aliasesList.add(als);
        DBObject als2 = new BasicDBObject();
        als2.put("id", "160");
        als2.put("cnt", "<sip:aa_live_" + rule.getId() + "@example.org;sipx-noroute=Voicemail?expires=30>;q=0.867");
        als2.put("rln", "userforward");
        aliasesList.add(als2);
        DBObject als3 = new BasicDBObject();
        als3.put("id", "6");
        als3.put("cnt", "sip:160@example.org");
        als3.put("rln", "alias");
        aliasesList.add(als3);
        DBObject als4 = new BasicDBObject();
        als4.put("id", "live");
        als4.put("cnt", "sip:160@example.org");
        als4.put("rln", "alias");
        aliasesList.add(als4);
        DBObject als5 = new BasicDBObject();
        als5.put("id", "+123456789");
        als5.put("cnt", "sip:160@example.org");
        als5.put("rln", "alias");
        aliasesList.add(als5);
        DBObject liveAttendantMongo = new BasicDBObject();
        liveAttendantMongo.put(ALIASES, aliasesList);
        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), liveAttendantMongo);

        // enable 'follow user call forwarding' option
        rule.setFollowUserCallForward(true);
        m_dialPlanContext.storeRule(rule);
        aliasesList = new ArrayList<DBObject>();
        als = new BasicDBObject();
        als.put("id", "160");
        als.put("cnt", "<sip:201@example.org;sipx-noroute=Voicemail?expires=10>;q=0.933");
        als.put("rln", "userforward");
        aliasesList.add(als);
        aliasesList.add(als2);
        aliasesList.add(als3);
        aliasesList.add(als4);
        aliasesList.add(als5);
        DBObject liveAttendantUserCallFwd = new BasicDBObject();
        liveAttendantUserCallFwd.put(ALIASES, aliasesList);
        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), liveAttendantUserCallFwd);

        // add live attendant schedule
        Schedule schedule = new GeneralSchedule();
        WorkingHours[] hours = new WorkingHours[1];
        WorkingTime wt = new WorkingTime();
        hours[0] = new WorkingHours();
        Calendar cal = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        cal.set(2014, Calendar.MARCH, 7, 10, 00);
        hours[0].setStart(cal.getTime());
        Integer startHour = Integer.valueOf(cal.get(Calendar.HOUR_OF_DAY));
        Integer startMinute = Integer.valueOf(cal.get(Calendar.MINUTE));
        cal.set(2014, Calendar.MARCH, 7, 11, 00);
        hours[0].setStop(cal.getTime());
        Integer stopHour = Integer.valueOf(cal.get(Calendar.HOUR_OF_DAY));
        Integer stopMinute = Integer.valueOf(cal.get(Calendar.MINUTE));
        hours[0].setEnabled(true);
        hours[0].setDay(ScheduledDay.FRIDAY);
        wt.setWorkingHours(hours);
        wt.setEnabled(true);
        schedule.setWorkingTime(wt);
        schedule.setName("live attendant schedule");
        m_forwardingContext.saveSchedule(schedule);
        rule.setSchedule(schedule);
        m_dialPlanContext.storeRule(rule);

        int offset = TimeZone.getDefault().getOffset((new Date()).getTime()) / 60000;
        Integer minutesFromSunday = (hours[0].getDay().getDayOfWeek() - 1) * 24 * 60;
        Integer startWithTimezone = minutesFromSunday + startHour * 60 + startMinute - offset;
        Integer stopWithTimezone = minutesFromSunday + stopHour * 60 + stopMinute - offset;
        String expected = Integer.toHexString(startWithTimezone) + ":" + Integer.toHexString(stopWithTimezone);

        aliasesList = new ArrayList<DBObject>();
        als = new BasicDBObject();
        als.put("id", "160");
        als.put("cnt", "<sip:201@example.org;sipx-noroute=Voicemail?expires=10>;q=0.933;sipx-ValidTime=\""
            + expected + "\"");
        als.put("rln", "userforward");
        aliasesList.add(als);
        aliasesList.add(als2);
        aliasesList.add(als3);
        aliasesList.add(als4);
        aliasesList.add(als5);
        DBObject liveAttendantSchedule = new BasicDBObject();
        liveAttendantSchedule.put(ALIASES, aliasesList);
        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), liveAttendantSchedule);

        // disable live attendant
        rule.setLiveAttendant(false);
        m_dialPlanContext.storeRule(rule);
        MongoTestCaseHelper.assertObjectWithFieldsValuesNotPresent(getEntityCollection(), new String[] {
            "ent"
        }, new String[] {
            "attendantrule"
        });
    }

    private DBCollection getEntityCollection() {
        return getImdb().getCollection("entity");
    }

    @Required
    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    @Required
    public void setDialPlanSetup(DialPlanSetup dialPlanSetup) {
        m_dialPlanSetup = dialPlanSetup;
    }

    @Required
    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }
}
