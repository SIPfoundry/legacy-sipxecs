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

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.Hibernate;
import org.hibernate.Query;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DSTChangeEvent;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.orm.hibernate3.HibernateTemplate;

/**
 * ForwardingContextImpl
 */
public class ForwardingContextImpl extends SipxHibernateDaoSupport implements ForwardingContext,
        ApplicationListener, DaoEventListener {
    private static final Log LOG = LogFactory.getLog(ForwardingContextImpl.class);
    private static final String PARAM_SCHEDULE_ID = "scheduleId";
    private static final String PARAM_USER_ID = "userId";
    private static final String PARAM_USER_GROUP_ID = "userGroupId";
    private static final String PARAM_FEATURE_ID = "featureId";
    private static final String PARAM_NAME = "name";
    private static final String SQL_CALLSEQUENCE_IDS = "select distinct u.user_id from users u";
    private CoreContext m_coreContext;
    private JdbcTemplate m_jdbcTemplate;
    private SipxReplicationContext m_sipxReplicationContext;

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
    @Override
    public CallSequence getCallSequenceForUser(User user) {
        return getCallSequenceForUserId(user.getId());
    }

    public void notifyCommserver(Collection<CallSequence> callSequences) {
        // Load call sequences, because aliases have been changed
        // TODO: replicate only call seq with those aliases
        for (CallSequence callSequence : callSequences) {
            getDaoEventPublisher().publishSave(callSequence);
        }
    }

    @Override
    public void saveCallSequence(CallSequence callSequence) {
        getHibernateTemplate().update(callSequence);
        m_coreContext.saveUser(callSequence.getUser());
    }

    @Override
    public CallSequence getCallSequenceForUserId(Integer userId) {
        HibernateTemplate hibernate = getHibernateTemplate();
        return hibernate.get(CallSequence.class, userId);
    }

    private void removeCallSequenceForUserId(Integer userId) {
        CallSequence callSequence = getCallSequenceForUserId(userId);
        callSequence.clear();
        getHibernateTemplate().update(callSequence);
        getDaoEventPublisher().publishDelete(callSequence);
        getHibernateTemplate().flush();
    }

    public void removeSchedulesForUserID(Integer userId) {
        List schedules = getPersonalSchedulesForUserId(userId);
        getHibernateTemplate().deleteAll(schedules);
    }

    @Override
    public Ring getRing(Integer id) {
        HibernateTemplate hibernate = getHibernateTemplate();
        return hibernate.load(Ring.class, id);
    }

    /**
     * Loads call sequences for all uses in current root organization
     *
     * @return list of CallSequence objects
     */
    private List<CallSequence> loadAllCallSequences() {
        List<CallSequence> callSequences = new ArrayList<CallSequence>();
        Query q = getHibernateTemplate().getSessionFactory().getCurrentSession()
        .createSQLQuery(SQL_CALLSEQUENCE_IDS).addScalar("user_id", Hibernate.INTEGER);
        List<Integer> ids = q.list();
        for (Integer id : ids) {
            CallSequence cs = getCallSequenceForUserId(id);
            if (CollectionUtils.isNotEmpty(cs.getRings())) {
                callSequences.add(cs);
            }
        }
        return callSequences;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    private Collection<CallSequence> getCallSequencesForGroup(Group group) {
        Collection<CallSequence> ids = new HashSet<CallSequence>();
        for (Integer id : m_coreContext.getGroupMembersIds(group)) {
            ids.add(getCallSequenceForUserId(id));
        }
        return ids;
    }

    @Override
    public List<Schedule> getPersonalSchedulesForUserId(Integer userId) {
        HibernateTemplate hibernate = getHibernateTemplate();

        return hibernate.findByNamedQueryAndNamedParam("userSchedulesForUserId", PARAM_USER_ID, userId);
    }

    public List<Ring> getRingsForScheduleId(Integer scheduleId) {
        HibernateTemplate hibernate = getHibernateTemplate();

        return hibernate.findByNamedQueryAndNamedParam("ringsForScheduleId", PARAM_SCHEDULE_ID, scheduleId);
    }

    private List<DialingRule> getDialingRulesForScheduleId(Integer scheduleId) {
        HibernateTemplate hibernate = getHibernateTemplate();

        return hibernate.findByNamedQueryAndNamedParam("dialingRulesForScheduleId", PARAM_SCHEDULE_ID, scheduleId);
    }

    @Override
    public Schedule getScheduleById(Integer scheduleId) {
        return getHibernateTemplate().load(Schedule.class, scheduleId);
    }

    @Override
    public void saveSchedule(Schedule schedule) {
        if (schedule.isNew()) {
            // check if new object
            checkForDuplicateNames(schedule);
            getHibernateTemplate().save(schedule);
        } else {
            // on edit action - check if the name for this schedule was modified
            // if the name was changed then perform duplicate name checking
            if (isNameChanged(schedule)) {
                checkForDuplicateNames(schedule);
            }
            getHibernateTemplate().merge(schedule);
            List<Ring> rings = getRingsForScheduleId(schedule.getId());
            Collection<CallSequence> css = new HashSet<CallSequence>();
            if (rings != null) {
                for (Ring ring : rings) {
                    css.add(ring.getCallSequence());
                }
            }
            notifyCommserver(css);
            getDaoEventPublisher().publishSave(schedule);
        }
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
        } else if (schedule instanceof FeatureSchedule) {
            count = getHibernateTemplate().findByNamedQueryAndNamedParam("anotherFeatureScheduleWithTheSameName",
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

    @Override
    public void deleteSchedulesById(Collection<Integer> scheduleIds) {
        Collection<Schedule> schedules = new ArrayList<Schedule>(scheduleIds.size());
        for (Integer id : scheduleIds) {
            Schedule schedule = getScheduleById(id);
            schedules.add(schedule);
            getDaoEventPublisher().publishDelete(schedule);
        }
        getHibernateTemplate().deleteAll(schedules);
    }

    @Override
    public List<UserGroupSchedule> getAllUserGroupSchedules() {
        return getHibernateTemplate().loadAll(UserGroupSchedule.class);
    }

    @Override
    public List<Schedule> getAllAvailableSchedulesForUser(User user) {
        List<Schedule> schedulesForUser = new ArrayList<Schedule>();
        schedulesForUser.addAll(getPersonalSchedulesForUserId(user.getId()));
        for (Group group : user.getGroups()) {
            schedulesForUser.addAll(getSchedulesForUserGroupId(group.getId()));
        }

        return schedulesForUser;
    }

    @Override
    public List<UserGroupSchedule> getSchedulesForUserGroupId(Integer userGroupId) {
        HibernateTemplate hibernate = getHibernateTemplate();

        return hibernate.findByNamedQueryAndNamedParam("userSchedulesForUserGroupId", PARAM_USER_GROUP_ID,
                userGroupId);
    }

    @Override
    public List<GeneralSchedule> getAllGeneralSchedules() {
        return getHibernateTemplate().loadAll(GeneralSchedule.class);
    }

    @Override
    public List<FeatureSchedule> getAllFeatureSchedules() {
        return getHibernateTemplate().loadAll(FeatureSchedule.class);
    }

    @Override
    public List<FeatureSchedule> getSchedulesForFeatureId(String featureId) {
        return getHibernateTemplate().findByNamedQueryAndNamedParam("schedulesForFeatureId", PARAM_FEATURE_ID,
            featureId);
    }

    @Override
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof DSTChangeEvent) {
            LOG.info("DST change event caught. Triggering alias generation.");
            m_sipxReplicationContext.generateAll(DataSet.ALIAS);
        }
    }

    /**
     * Only used from WEB UI test code
     */
    @Override
    public void clear() {
        Collection<CallSequence> sequences = loadAllCallSequences();
        for (CallSequence sequence : sequences) {
            sequence.clear();
            saveCallSequence(sequence);
        }
    }

    @Override
    public void clearSchedules() {
        Collection<Schedule> schedules = getHibernateTemplate().loadAll(Schedule.class);
        getHibernateTemplate().deleteAll(schedules);
    }

    @Override
    public boolean isCallSequenceReplicable(User user) {
        int any = m_jdbcTemplate.queryForInt("select count(*) from ring where user_id = ?", user.getId());
        return any > 0;
    }

    public void setConfigJdbcTemplate(JdbcTemplate jdbcTemplate) {
        m_jdbcTemplate = jdbcTemplate;
    }

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof Schedule) {
            Schedule schedule = (Schedule) entity;
            if (schedule instanceof GeneralSchedule) {
                // get all dialing rules and set schedule to Always
                List<DialingRule> rules = getDialingRulesForScheduleId(schedule.getId());
                if (rules != null) {
                    for (DialingRule rule : rules) {
                        rule.setSchedule(null);
                    }
                    getHibernateTemplate().saveOrUpdateAll(rules);
                    for (DialingRule rule : rules) {
                        if (rule instanceof AttendantRule) {
                            AttendantRule aaRule = (AttendantRule) rule;
                            m_sipxReplicationContext.generate(aaRule);
                        } else {
                            getDaoEventPublisher().publishSave(rule);
                        }
                    }
                }
            } else if (schedule instanceof UserSchedule || schedule instanceof UserGroupSchedule) {
                Collection<CallSequence> css = new HashSet<CallSequence>();
                // get all rings and set schedule to Always
                List<Ring> rings = getRingsForScheduleId(schedule.getId());
                if (rings != null) {
                    for (Ring ring : rings) {
                        ring.setSchedule(null);
                        css.add(ring.getCallSequence());
                    }
                    getHibernateTemplate().saveOrUpdateAll(rings);
                }
                notifyCommserver(css);
            }
        } else if (entity instanceof User) {
            User user = (User) entity;
            removeCallSequenceForUserId(user.getId());
            removeSchedulesForUserID(user.getId());
        }
    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof CallSequence) {
            CallSequence seq = (CallSequence) entity;
            m_sipxReplicationContext.generate(seq.getUser());
        } else if (entity instanceof Schedule) {
            Schedule schedule = (Schedule) entity;
            if (schedule instanceof GeneralSchedule) {
                List<DialingRule> rules = getDialingRulesForScheduleId(schedule.getId());
                if (rules != null) {
                    for (DialingRule rule : rules) {
                        if (rule instanceof AttendantRule) {
                            AttendantRule aaRule = (AttendantRule) rule;
                            m_sipxReplicationContext.generate(aaRule);
                        } else {
                            getDaoEventPublisher().publishSave(rule);
                        }
                    }
                }
            }
        }
    }
}
