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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DSTChangeEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;
import org.sipfoundry.sipxconfig.common.event.UserGroupDeleteListener;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

/**
 * ForwardingContextImpl
 * 
 */
public class ForwardingContextImpl extends HibernateDaoSupport implements ForwardingContext,
        ApplicationListener {

    private static final String PARAM_SCHEDULE_ID = "scheduleId";
    private static final String PARAM_USER_ID = "userId";
    private static final String PARAM_USER_GROUP_ID = "userGroupId";
    private static final String ID = "id";
    private static final String NAME = "name";

    private CoreContext m_coreContext;

    private SipxReplicationContext m_replicationContext;

    /**
     * Looks for a call sequence associated with a given user.
     * 
     * This version just assumes that CallSequence id is the same as user id. More general
     * implementation would run a query. <code>
     *      String ringsForUser = "from CallSequence cs where cs.user = :user";
     *      hibernate.findByNamedParam(ringsForUser, "user", user);
     * </code>
     * 
     * @param user for which CallSequence object is retrieved
     */
    public CallSequence getCallSequenceForUser(User user) {
        return getCallSequenceForUserId(user.getId());
    }

    public void notifyCommserver() {
        // Notify commserver of ALIAS
        m_replicationContext.generate(DataSet.ALIAS);
    }

    public void saveCallSequence(CallSequence callSequence) {
        getHibernateTemplate().update(callSequence);
        // Notify commserver of ALIAS
        notifyCommserver();
    }

    public void flush() {
        getHibernateTemplate().flush();
    }

    public CallSequence getCallSequenceForUserId(Integer userId) {
        HibernateTemplate hibernate = getHibernateTemplate();
        return (CallSequence) hibernate.load(CallSequence.class, userId);
    }

    public void removeCallSequenceForUserId(Integer userId) {
        CallSequence callSequence = getCallSequenceForUserId(userId);
        callSequence.clear();
        saveCallSequence(callSequence);
        getHibernateTemplate().flush();
    }

    public void removeSchedulesForUserID(Integer userId) {
        List schedules = getPersonalSchedulesForUserId(userId);
        getHibernateTemplate().deleteAll(schedules);
    }

    public Ring getRing(Integer id) {
        HibernateTemplate hibernate = getHibernateTemplate();
        return (Ring) hibernate.load(Ring.class, id);
    }

    public Collection getAliasMappings() {
        List aliases = new ArrayList();
        List sequences = loadAllCallSequences();
        for (Iterator i = sequences.iterator(); i.hasNext();) {
            CallSequence sequence = (CallSequence) i.next();
            aliases.addAll(sequence.generateAliases(m_coreContext.getDomainName()));
        }
        return aliases;
    }

    /**
     * Loads call sequences for all uses in current root organization
     * 
     * @return list of CallSequence objects
     */
    private List loadAllCallSequences() {
        List sequences = getHibernateTemplate().find("from CallSequence cs");
        return sequences;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    public UserDeleteListener createUserDeleteListener() {
        return new OnUserDelete();
    }

    public void clear() {
        Collection sequences = loadAllCallSequences();
        for (Iterator i = sequences.iterator(); i.hasNext();) {
            CallSequence sequence = (CallSequence) i.next();
            sequence.clear();
            saveCallSequence(sequence);
        }
    }

    public UserGroupDeleteListener createUserGroupDeleteListener() {
        return new OnUserGroupDelete();
    }

    private class OnUserDelete extends UserDeleteListener {
        protected void onUserDelete(User user) {
            removeCallSequenceForUserId(user.getId());
            removeSchedulesForUserID(user.getId());
        }
    }

    private class OnUserGroupDelete extends UserGroupDeleteListener {
        protected void onUserGroupDelete(Group group) {
            List<UserGroupSchedule> schedules = getSchedulesForUserGroupId(group.getId());
            if (schedules != null && schedules.size() > 0) {
                // get all rings for the schedules, set all on always
                List<Ring> ringsToModify = new ArrayList<Ring>();
                for (UserGroupSchedule ugSchedule : schedules) {
                    List<Ring> rings = getRingsForScheduleId(ugSchedule.getId());
                    for (Ring ring : rings) {
                        // set schedule on always
                        ring.setSchedule(null);
                        ringsToModify.add(ring);
                    }
                }
                getHibernateTemplate().saveOrUpdateAll(ringsToModify);
                getHibernateTemplate().deleteAll(schedules);
                notifyCommserver();
            }
        }
    }

    public List<Schedule> getPersonalSchedulesForUserId(Integer userId) {
        HibernateTemplate hibernate = getHibernateTemplate();

        return hibernate.findByNamedQueryAndNamedParam("userSchedulesForUserId", PARAM_USER_ID,
                userId);
    }

    public List getRingsForScheduleId(Integer scheduleId) {
        HibernateTemplate hibernate = getHibernateTemplate();

        return hibernate.findByNamedQueryAndNamedParam("ringsForScheduleId", PARAM_SCHEDULE_ID,
                scheduleId);
    }

    private List getDialingRulesForScheduleId(Integer scheduleId) {
        HibernateTemplate hibernate = getHibernateTemplate();

        return hibernate.findByNamedQueryAndNamedParam("dialingRulesForScheduleId",
                PARAM_SCHEDULE_ID, scheduleId);
    }

    public Schedule getScheduleById(Integer scheduleId) {
        return (Schedule) getHibernateTemplate().load(Schedule.class, scheduleId);
    }

    public void saveSchedule(Schedule schedule) {
        if (schedule.isNew()) {
            // check if new object
            checkForDuplicateNames(schedule);
        } else {
            // on edit action - check if the name for this schedule was modified
            // if the name was changed then perform duplicate name checking
            if (isNameChanged(schedule)) {
                checkForDuplicateNames(schedule);
            }
        }
        getHibernateTemplate().saveOrUpdate(schedule);
        // Notify commserver of ALIAS
        notifyCommserver();
    }

    private void checkForDuplicateNames(Schedule schedule) {
        if (isNameInUse(schedule)) {
            throw new UserException("A schedule with name {0} is already defined", schedule
                    .getName());
        }
    }

    private boolean isNameInUse(Schedule schedule) {
        String query = "";
        Integer entityId = null;
        if (schedule instanceof UserSchedule) {
            query = "anotherUserScheduleWithTheSameName";
            entityId = schedule.getUser().getId();
        } else if (schedule instanceof UserGroupSchedule) {
            query = "anotherUserGroupScheduleWithTheSameName";
            entityId = schedule.getUserGroup().getId();
        }
        List count = getHibernateTemplate().findByNamedQueryAndNamedParam(query, new String[] {
            ID, NAME
        }, new Object[] {
            entityId, schedule.getName()
        });

        return DataAccessUtils.intResult(count) > 0;
    }

    private boolean isNameChanged(Schedule schedule) {
        List count = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "countScheduleWithSameName", new String[] {
                    ID, NAME
                }, new Object[] {
                    schedule.getId(), schedule.getName()
                });

        return DataAccessUtils.intResult(count) == 0;
    }

    public List<Schedule> deleteSchedulesById(Collection<Integer> scheduleIds) {
        if (scheduleIds.isEmpty()) {
            // no schedules to delete => nothing to do
            return null;
        }
        List<Schedule> unassignedSchedules = new ArrayList<Schedule>();
        List<Schedule> assignedSchedules = new ArrayList<Schedule>();
        for (Integer id : scheduleIds) {
            Schedule schedule = getScheduleById(id);
            List deps = null;
            if (schedule instanceof GeneralSchedule) {
                deps = getDialingRulesForScheduleId(id);
            } else {
                deps = getRingsForScheduleId(id);
            }
            if (deps == null || deps.isEmpty()) {
                unassignedSchedules.add(schedule);
            } else {
                assignedSchedules.add(schedule);
            }
        }
        getHibernateTemplate().deleteAll(unassignedSchedules);
        return assignedSchedules;
    }

    public List<UserGroupSchedule> getAllUserGroupSchedules() {
        return getHibernateTemplate().loadAll(UserGroupSchedule.class);
    }

    public List<Schedule> getAllAvailableSchedulesForUser(User user) {
        List<Schedule> schedulesForUser = new ArrayList<Schedule>();
        schedulesForUser.addAll(getPersonalSchedulesForUserId(user.getId()));
        for (Group group : user.getGroups()) {
            schedulesForUser.addAll(getSchedulesForUserGroupId(group.getId()));
        }

        return schedulesForUser;
    }

    public List<UserGroupSchedule> getSchedulesForUserGroupId(Integer userGroupId) {
        HibernateTemplate hibernate = getHibernateTemplate();

        return hibernate.findByNamedQueryAndNamedParam("userSchedulesForUserGroupId",
                PARAM_USER_GROUP_ID, userGroupId);
    }

    public List<GeneralSchedule> getAllGeneralSchedules() {
        return getHibernateTemplate().loadAll(GeneralSchedule.class);
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof DSTChangeEvent) {
            notifyCommserver();
        }
    }
}
