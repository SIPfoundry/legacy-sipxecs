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
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DSTChangeEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventPublisher;
import org.sipfoundry.sipxconfig.common.event.ScheduleDeleteListener;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;
import org.sipfoundry.sipxconfig.common.event.UserGroupSaveDeleteListener;
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
public class ForwardingContextImpl extends HibernateDaoSupport implements ForwardingContext, ApplicationListener {

    private static final String PARAM_SCHEDULE_ID = "scheduleId";
    private static final String PARAM_USER_ID = "userId";
    private static final String PARAM_USER_GROUP_ID = "userGroupId";
    private static final String PARAM_NAME = "name";

    private CoreContext m_coreContext;

    private SipxReplicationContext m_replicationContext;

    private DaoEventPublisher m_daoEventPublisher;

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
        m_replicationContext.generate(DataSet.USER_FORWARD);
    }

    public void saveCallSequence(CallSequence callSequence) {
        getHibernateTemplate().update(callSequence);
        m_coreContext.saveUser(callSequence.getUser());
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
    private List<CallSequence> loadAllCallSequences() {
        List sequences = getHibernateTemplate().find("from CallSequence cs");
        return sequences;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    public void setDaoEventPublisher(DaoEventPublisher daoEventPublisher) {
        m_daoEventPublisher = daoEventPublisher;
    }

    public UserDeleteListener createUserDeleteListener() {
        return new OnUserDelete();
    }

    public UserGroupSaveDeleteListener createUserGroupSaveDeleteListener() {
        return new OnUserGroupDelete();
    }

    public ScheduleDeleteListener createScheduleDeleteListener() {
        return new OnScheduleDelete();
    }

    private class OnUserDelete extends UserDeleteListener {
        @Override
        protected void onUserDelete(User user) {
            removeCallSequenceForUserId(user.getId());
            removeSchedulesForUserID(user.getId());
        }
    }

    private class OnUserGroupDelete extends UserGroupSaveDeleteListener {
        @Override
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
            }
            notifyCommserver();
        }

        @Override
        protected void onUserGroupSave(Group group) {
            notifyCommserver();
        }
    }

    private class OnScheduleDelete extends ScheduleDeleteListener {
        @Override
        protected void onScheduleDelete(Schedule schedule) {
            if (schedule instanceof GeneralSchedule) {
                // get all dialing rules and set schedule to Always
                List<DialingRule> rules = getDialingRulesForScheduleId(schedule.getId());
                if (rules != null) {
                    for (DialingRule rule : rules) {
                        rule.setSchedule(null);
                    }
                    getHibernateTemplate().saveOrUpdateAll(rules);
                }
            } else {
                // get all rings and set schedule to Always
                List<Ring> rings = getRingsForScheduleId(schedule.getId());
                if (rings != null) {
                    for (Ring ring : rings) {
                        ring.setSchedule(null);
                    }
                    getHibernateTemplate().saveOrUpdateAll(rings);
                }
            }
            notifyCommserver();
        }
    }

    public List<Schedule> getPersonalSchedulesForUserId(Integer userId) {
        HibernateTemplate hibernate = getHibernateTemplate();

        return hibernate.findByNamedQueryAndNamedParam("userSchedulesForUserId", PARAM_USER_ID, userId);
    }

    public List getRingsForScheduleId(Integer scheduleId) {
        HibernateTemplate hibernate = getHibernateTemplate();

        return hibernate.findByNamedQueryAndNamedParam("ringsForScheduleId", PARAM_SCHEDULE_ID, scheduleId);
    }

    private List getDialingRulesForScheduleId(Integer scheduleId) {
        HibernateTemplate hibernate = getHibernateTemplate();

        return hibernate.findByNamedQueryAndNamedParam("dialingRulesForScheduleId", PARAM_SCHEDULE_ID, scheduleId);
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
            throw new UserException("A schedule with name {0} is already defined", schedule.getName());
        }
    }

    private boolean isNameInUse(Schedule schedule) {
        List count = null;
        if (schedule instanceof UserSchedule) {
            count = getHibernateTemplate().findByNamedQueryAndNamedParam("anotherUserScheduleWithTheSameName",
                    new String[] {
                        PARAM_USER_ID, PARAM_NAME
                    }, new Object[] {
                        schedule.getUser().getId(), schedule.getName()
                    });
        } else if (schedule instanceof UserGroupSchedule) {
            count = getHibernateTemplate().findByNamedQueryAndNamedParam("anotherUserGroupScheduleWithTheSameName",
                    new String[] {
                        PARAM_USER_GROUP_ID, PARAM_NAME
                    }, new Object[] {
                        schedule.getUserGroup().getId(), schedule.getName()
                    });
        } else if (schedule instanceof GeneralSchedule) {
            count = getHibernateTemplate().findByNamedQueryAndNamedParam("anotherGeneralScheduleWithTheSameName",
                    PARAM_NAME, schedule.getName());
        }

        return DataAccessUtils.intResult(count) > 0;
    }

    private boolean isNameChanged(Schedule schedule) {
        List count = getHibernateTemplate().findByNamedQueryAndNamedParam("countScheduleWithSameName", new String[] {
            PARAM_SCHEDULE_ID, PARAM_NAME
        }, new Object[] {
            schedule.getId(), schedule.getName()
        });

        return DataAccessUtils.intResult(count) == 0;
    }

    public void deleteSchedulesById(Collection<Integer> scheduleIds) {
        Collection<Schedule> schedules = new ArrayList<Schedule>(scheduleIds.size());
        for (Integer id : scheduleIds) {
            Schedule schedule = getScheduleById(id);
            schedules.add(schedule);
            m_daoEventPublisher.publishDelete(schedule);
        }
        getHibernateTemplate().deleteAll(schedules);
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

        return hibernate.findByNamedQueryAndNamedParam("userSchedulesForUserGroupId", PARAM_USER_GROUP_ID,
                userGroupId);
    }

    public List<GeneralSchedule> getAllGeneralSchedules() {
        return getHibernateTemplate().loadAll(GeneralSchedule.class);
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof DSTChangeEvent) {
            notifyCommserver();
        }
    }

    /**
     * Only used from WEB UI test code
     */
    public void clear() {
        Collection<CallSequence> sequences = loadAllCallSequences();
        for (CallSequence sequence : sequences) {
            sequence.clear();
            saveCallSequence(sequence);
        }
    }

    public void clearSchedules() {
        Collection<Schedule> schedules = getHibernateTemplate().loadAll(Schedule.class);
        getHibernateTemplate().deleteAll(schedules);
    }
}
