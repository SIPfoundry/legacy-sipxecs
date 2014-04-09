/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.dialplan;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.TimeZone;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.NameInUseException;
import org.sipfoundry.sipxconfig.common.ScheduledDay;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.attendant.AutoAttendantSettings;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime.Interval;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime.WorkingHours;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.forwarding.Schedule;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.localization.LanguageUpdatedEvent;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationListener;
import org.springframework.dao.support.DataAccessUtils;

public class AutoAttendantManagerImpl extends SipxHibernateDaoSupport<AutoAttendant> implements
        AutoAttendantManager, BeanFactoryAware, ApplicationListener<LanguageUpdatedEvent> {
    private static final String AUTO_ATTENDANT = "auto attendant";
    private static final String DID = "did";
    private static final Log LOG = LogFactory.getLog(AutoAttendantManagerImpl.class);
    private AliasManager m_aliasManager;
    private SettingDao m_settingDao;
    private BeanFactory m_beanFactory;
    private FeatureManager m_featureManager;
    private MediaServer m_mediaServer;
    private BeanWithSettingsDao<AutoAttendantSettings> m_beanWithSettingsDao;

    @Override
    public boolean isAliasInUse(String alias) {
        AutoAttendantSettings settings = getSettings();
        return !getAutoAttendantsWithName(alias).isEmpty()
                || StringUtils.equalsIgnoreCase(alias, settings.getDisablePrefix())
                || StringUtils.equalsIgnoreCase(alias, settings.getEnablePrefix())
                || StringUtils.equalsIgnoreCase(alias, settings.getLiveDid());
    }

    @Override
    public void storeAutoAttendant(AutoAttendant aa) {
        // Check for duplicate names or extensions before saving the call group
        String name = aa.getName();
        if (!m_aliasManager.canObjectUseAlias(aa, name)) {
            throw new NameInUseException(AUTO_ATTENDANT, name);
        }

        // check if valid reg ex for allow / deny dialing extensions
        checkRegEx(aa.getDenyDial(), "&error.invalid.denyDialExpression");
        checkRegEx(aa.getAllowDial(), "&error.invalid.allowDialExpression");

        clearUnsavedValueStorage(aa.getValueStorage());
        if (aa.isNew()) {
            getHibernateTemplate().save(aa);
        } else {
            getHibernateTemplate().merge(aa);
        }
        getHibernateTemplate().flush();
        getDaoEventPublisher().publishSave(aa);
    }

    private static void checkRegEx(String regEx, String errMessage) {
        if (StringUtils.isNotEmpty(regEx)) {
            try {
                Pattern.compile(regEx);
            } catch (PatternSyntaxException ex) {
                throw new UserException(errMessage);
            }
        }
    }

    @Override
    public AutoAttendant getOperator() {
        return getAttendant(AutoAttendant.OPERATOR_ID);
    }

    private AutoAttendant getAfterhour() {
        return getAttendant(AutoAttendant.AFTERHOUR_ID);
    }

    private AutoAttendant getAttendant(String systemId) {
        String query = "from AutoAttendant a where a.systemId = :systemId";
        List<AutoAttendant> operatorList = getHibernateTemplate().findByNamedParam(query, "systemId", systemId);

        return DaoUtils.requireOneOrZero(operatorList, query);
    }

    @Override
    public List<AutoAttendant> getAutoAttendants() {
        List<AutoAttendant> aas = getHibernateTemplate().loadAll(AutoAttendant.class);
        return aas;
    }

    @Override
    public AutoAttendant getAutoAttendant(Integer id) {
        return getHibernateTemplate().load(AutoAttendant.class, id);
    }

    @Override
    public AutoAttendant getAutoAttendantBySystemName(String systemId) {
        Integer id = AutoAttendant.getIdFromSystemId(systemId);
        if (id != null) {
            return getAutoAttendant(id);
        }
        return getAttendant(systemId);
    }

    @Override
    public void deleteAutoAttendantsByIds(Collection<Integer> attendantIds) {
        for (Integer id : attendantIds) {
            AutoAttendant aa = getAutoAttendant(id);
            getDaoEventPublisher().publishDelete(aa);
            deleteAutoAttendant(aa);
        }
    }

    @Override
    public void dupeAutoAttendants(Collection<Integer> attendantsIds) {
        for (Integer id : attendantsIds) {
            AutoAttendant aa = getAutoAttendant(id);
            AutoAttendant newAa = newAutoAttendantWithDefaultGroup();
            newAa.setName(aa.getName() + "-duped" + System.currentTimeMillis());
            newAa.setDescription("autogenerated AA");
            aa.duplicateSettings(newAa);
            storeAutoAttendant(newAa);
        }
    }

    @Override
    public void deleteAutoAttendant(AutoAttendant attendant) {
        if (attendant.isPermanent()) {
            throw new AttendantInUseException();
        }

        attendant.setValueStorage(clearUnsavedValueStorage(attendant.getValueStorage()));
        getHibernateTemplate().refresh(attendant);

        Collection<AttendantRule> attendantRules = getHibernateTemplate().loadAll(AttendantRule.class);
        Collection<DialingRule> affectedRules = new ArrayList<DialingRule>();
        for (AttendantRule rule : attendantRules) {
            if (rule.checkAttendant(attendant)) {
                affectedRules.add(rule);
            }
        }
        if (!affectedRules.isEmpty()) {
            List<String> names = new ArrayList<String>(affectedRules.size());
            for (Iterator<DialingRule> i = affectedRules.iterator(); i.hasNext();) {
                DialingRule rule = i.next();
                names.add(rule.getName());
            }
            String ruleNames = StringUtils.join(names.iterator(), ", ");
            throw new AttendantInUseException(new Object[] {
                ruleNames
            });
        }

        // deselect special mode if removign attendant
        AttendantSpecialMode specialMode = loadAttendantSpecialMode();
        if (attendant.equals(specialMode.getAttendant())) {
            specialMode.setAttendant(null);
            specialMode.setEnabled(false);
            if (specialMode.isNew()) {
                getHibernateTemplate().save(specialMode);
            } else {
                getHibernateTemplate().merge(specialMode);
            }
        }

        getHibernateTemplate().delete(attendant);
    }

    @Override
    public Group getDefaultAutoAttendantGroup() {
        return m_settingDao.getGroupCreateIfNotFound(ATTENDANT_GROUP_ID, "default");
    }

    @Override
    public AutoAttendant newAutoAttendantWithDefaultGroup() {
        AutoAttendant aa = m_beanFactory.getBean(AutoAttendant.BEAN_NAME, AutoAttendant.class);

        // All auto attendants share same group: default
        Set groups = aa.getGroups();
        if (groups == null || groups.size() == 0) {
            aa.addGroup(getDefaultAutoAttendantGroup());
        }

        return aa;
    }

    public Setting getAttendantSettingModel() {
        AutoAttendant aa = m_beanFactory.getBean(AutoAttendant.BEAN_NAME, AutoAttendant.class);
        return aa.getSettings();
    }

    public AutoAttendant createSystemAttendant(String systemId) {
        AutoAttendant aa = (AutoAttendant) m_beanFactory.getBean(systemId + "Prototype");
        aa.resetToFactoryDefault();
        return aa;
    }

    private Collection getAutoAttendantsWithName(String alias) {
        return getHibernateTemplate().findByNamedQueryAndNamedParam("autoAttendantIdsWithName", "value", alias);
    }

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection autoAttendants = getAutoAttendantsWithName(alias);
        return BeanId.createBeanIdCollection(autoAttendants, AutoAttendant.class);
    }

    @Override
    public AutoAttendant createOperator(String attendantId) {
        AutoAttendant attendant = getAttendant(attendantId);
        if (attendant != null) {
            // already exists... - nothing created
            return null;
        }
        attendant = createSystemAttendant(attendantId);
        attendant.addGroup(getDefaultAutoAttendantGroup());
        if (attendant.isNew()) {
            getHibernateTemplate().save(attendant);
        } else {
            getHibernateTemplate().merge(attendant);
        }
        getDaoEventPublisher().publishSave(attendant);
        return attendant;
    }

    @Override
    public List< ? extends DialingRule> getDialingRules(Location location) {
        if (!m_featureManager.isFeatureEnabled(Ivr.FEATURE)) {
            return Collections.emptyList();
        }
        AutoAttendantSettings settings = getSettings();

        String prefixFormat = "%s.";
        DialingRule liveAaRule = new MappingRule.LiveAttendantManagement(m_mediaServer, settings.getLiveDid(),
                String.format(prefixFormat, settings.getDisablePrefix()), String.format(prefixFormat,
                        settings.getEnablePrefix()), location);
        DialingRule[] rules = new DialingRule[] {
            liveAaRule
        };
        return Arrays.asList(rules);
    }

    /**
     * This is for testing only.
     */
    @Override
    public void clear() {
        List<AutoAttendant> attendants = getHibernateTemplate().loadAll(AutoAttendant.class);
        getDaoEventPublisher().publishDelete(attendants);
        getHibernateTemplate().deleteAll(attendants);
    }

    @Override
    public void updatePrompts(File sourceDir) {
        try {
            if (getOperator() != null) {
                getOperator().updatePrompt(sourceDir);
            }
            if (getAfterhour() != null) {
                getAfterhour().updatePrompt(sourceDir);
            }
        } catch (IOException e) {
            LOG.warn("Failed to copy default AA prompts to AA prompts directory");
        }
    }

    @Override
    public void deselectSpecial(AutoAttendant aa) {
        AttendantSpecialMode specialMode = loadAttendantSpecialMode();
        if (specialMode.isEnabled()) {
            LOG.warn("Cannot de select new special attendnat when special attendant active");
            return;
        }

        specialMode.setAttendant(null);
        if (specialMode.isNew()) {
            getHibernateTemplate().save(specialMode);
        } else {
            getHibernateTemplate().merge(specialMode);
        }
        getDaoEventPublisher().publishSave(specialMode);
    }

    @Override
    public AutoAttendant getSelectedSpecialAttendant() {
        AttendantSpecialMode specialMode = loadAttendantSpecialMode();
        return specialMode.getAttendant();
    }

    @Override
    public boolean getSpecialMode() {
        AttendantSpecialMode specialMode = loadAttendantSpecialMode();
        return specialMode.isEnabled();
    }

    @Override
    public void setAttendantSpecialMode(boolean enabled, AutoAttendant aa) {
        AttendantSpecialMode specialMode = loadAttendantSpecialMode();
        if (specialMode == null) {
            specialMode = new AttendantSpecialMode();
        }
        specialMode.setEnabled(enabled);
        if (enabled && specialMode.getAttendant() == null) {
            specialMode.setAttendant(getAfterhour());
        } else {
            specialMode.setAttendant(aa);
        }
        if (specialMode.isNew()) {
            getHibernateTemplate().save(specialMode);
        } else {
            getHibernateTemplate().merge(specialMode);
        }
        getDaoEventPublisher().publishSave(specialMode);
    }

    private AttendantSpecialMode loadAttendantSpecialMode() {
        List<AttendantSpecialMode> asm = getHibernateTemplate().loadAll(AttendantSpecialMode.class);
        AttendantSpecialMode specialMode = DataAccessUtils.singleResult(asm);
        return specialMode;
    }

    @Override
    public AutoAttendantSettings getSettings() {
        return m_beanWithSettingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(AutoAttendantSettings settings) {
        String did = settings.getLiveDid();
        if (!m_aliasManager.canObjectUseAlias(settings, did)) {
            throw new NameInUseException(DID, did);
        }
        m_beanWithSettingsDao.upsert(settings);
    }

    @Override
    public boolean manageLiveAttendant(String code, boolean enable) {
        try {
            Collection<AttendantRule> rules = getHibernateTemplate().findByNamedQueryAndNamedParam("aaRulesForCode",
                    "code", code);
            AttendantRule rule = DaoUtils.requireOneOrZero(rules, "aaByCode");
            rule.setLiveAttendantEnabled(enable);
            if (!enable) {
                // calculate expiration time, if no time specified then revert at schedule change
                Integer hoursToExpire = getSettings().getExpireTime();
                Date expire = null;
                if (hoursToExpire != null) {
                    Calendar calendar = Calendar.getInstance(TimeZone.getDefault());
                    calendar.setTime(new Date());
                    calendar.add(Calendar.HOUR, hoursToExpire);
                    expire = calendar.getTime();
                    rule.setLiveAttendantExpire(expire);
                } else {
                    Schedule schedule = rule.getSchedule();
                    if (schedule != null) {
                        WorkingHours[] hours = new WorkingHours[1];
                        WorkingTime wt = new WorkingTime();
                        hours[0] = new WorkingHours();
                        TimeZone utc = TimeZone.getTimeZone("UTC");
                        Calendar cal = Calendar.getInstance(utc);
                        hours[0].setStart(cal.getTime());
                        hours[0].setStop(cal.getTime());
                        hours[0].setDay(ScheduledDay.getScheduledDay(cal.get(Calendar.DAY_OF_WEEK)));
                        wt.setWorkingHours(hours);
                        List<Interval> intervals = wt.calculateValidTime(utc);
                        int intervalNow = intervals.get(0).getStart();
                        List<Interval> scheduleIntervals = schedule.getWorkingTime().calculateValidTime(
                                TimeZone.getDefault());
                        int dif = 0;
                        int firstStartInWeek = 0;
                        for (Interval interval : scheduleIntervals) {
                            if (interval.getStart() < firstStartInWeek || firstStartInWeek == 0) {
                                firstStartInWeek = interval.getStart();
                            }
                            int intervalDifference = interval.getStart() - intervalNow;
                            if (intervalDifference > 0 && (dif == 0 || intervalDifference < dif)) {
                                dif = intervalDifference;
                            }
                        }
                        if (dif != 0) {
                            // there's an interval to be considered this week
                            Calendar calendar = Calendar.getInstance(TimeZone.getDefault());
                            calendar.setTime(new Date());
                            calendar.add(Calendar.MINUTE, dif);
                            expire = calendar.getTime();
                        } else {
                            // if no interval to reenable for this week then roll to next sunday
                            // and add first start time
                            Calendar calendar = Calendar.getInstance(utc);
                            calendar.setTime(new Date());
                            int weekday = calendar.get(Calendar.DAY_OF_WEEK);
                            int days = Calendar.SUNDAY - weekday;
                            if (days < 0) {
                                days += 7;
                            }
                            calendar.add(Calendar.DAY_OF_YEAR, days);
                            calendar.set(Calendar.HOUR, 0);
                            calendar.set(Calendar.MINUTE, 0);
                            calendar.set(Calendar.SECOND, 0);
                            calendar.add(Calendar.MINUTE, firstStartInWeek);
                            expire = calendar.getTime();
                        }
                        rule.setLiveAttendantExpire(expire);
                    }
                    LOG.debug("Reenable live attendant for code " + code + " at " + expire);
                }
            } else {
                rule.setLiveAttendantExpire(null);
            }
            getHibernateTemplate().save(rule);
            getDaoEventPublisher().publishSave(rule);
            return true;
        } catch (Exception ex) {
            LOG.error(String.format("Cannot change live attendant for code %s, cause %s ", code, ex.getMessage()));
        }
        return false;
    }

    public void checkLiveAttendant() {
        LOG.trace("Check live attendant expiration");
        // hibernate query to load all Live AA rules enabled and night on disable
        // check if current time after expiration time, if so then re enable rule
        Collection<AttendantRule> rules = getHibernateTemplate().findByNamedQuery("disabledLiveAaRules");
        for (AttendantRule rule : rules) {
            LOG.debug("found rule " + rule.getExtension() + " will expire at " + rule.getLiveAttendantExpire());
            if (rule.getLiveAttendantExpire() != null && new Date().after(rule.getLiveAttendantExpire())) {
                LOG.info("Expiration passed, reenable " + rule.getExtension());
                rule.setLiveAttendantEnabled(true);
                rule.setLiveAttendantExpire(null);
                getHibernateTemplate().merge(rule);
                getDaoEventPublisher().publishSave(rule);
            }
        }
    }

    @Required
    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    @Required
    public void setBeanWithSettingsDao(BeanWithSettingsDao<AutoAttendantSettings> settingDao) {
        m_beanWithSettingsDao = settingDao;
    }

    @Required
    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Required
    public void setMediaServer(MediaServer mediaServer) {
        m_mediaServer = mediaServer;
    }

    @Override
    public void onApplicationEvent(LanguageUpdatedEvent event) {
        LOG.debug("Language updated, updating prompts...");
        updatePrompts(new File(event.getPromptsDir(), event.getCurrentLanguageDir()));
    }

    @Override
    public AutoAttendant getAutoAttendantByName(String attendantName) {
        String query = "from AutoAttendant a where a.name = :name";
        List<AutoAttendant> operatorList = getHibernateTemplate().findByNamedParam(query, "name", attendantName);

        return DaoUtils.requireOneOrZero(operatorList, query);
    }

}
