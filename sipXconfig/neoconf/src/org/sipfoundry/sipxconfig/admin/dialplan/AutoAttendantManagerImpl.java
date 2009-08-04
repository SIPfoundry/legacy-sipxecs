/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;

public class AutoAttendantManagerImpl extends SipxHibernateDaoSupport implements AutoAttendantManager,
        BeanFactoryAware {

    private static final String AUTO_ATTENDANT = "auto attendant";

    private static final Log LOG = LogFactory.getLog(AutoAttendantManagerImpl.class);

    private ServiceConfigurator m_serviceConfigurator;

    private SipxServiceManager m_sipxServiceManager;

    private AliasManager m_aliasManager;

    private SettingDao m_settingDao;

    private BeanFactory m_beanFactory;

    public boolean isAliasInUse(String alias) {
        return !getAutoAttendantsWithName(alias).isEmpty();
    }

    public void storeAutoAttendant(AutoAttendant aa) {
        // Check for duplicate names or extensions before saving the call group
        String name = aa.getName();
        if (!m_aliasManager.canObjectUseAlias(aa, name)) {
            throw new NameInUseException(AUTO_ATTENDANT, name);
        }

        clearUnsavedValueStorage(aa.getValueStorage());
        getHibernateTemplate().saveOrUpdate(aa);
        getHibernateTemplate().flush();

        replicateConfig();
    }

    public AutoAttendant getOperator() {
        return getAttendant(AutoAttendant.OPERATOR_ID);
    }

    private AutoAttendant getAfterhour() {
        return getAttendant(AutoAttendant.AFTERHOUR_ID);
    }

    private AutoAttendant getAttendant(String systemId) {
        String query = "from AutoAttendant a where a.systemId = :systemId";
        List operatorList = getHibernateTemplate().findByNamedParam(query, "systemId", systemId);

        return (AutoAttendant) DaoUtils.requireOneOrZero(operatorList, query);
    }

    public List<AutoAttendant> getAutoAttendants() {
        List<AutoAttendant> aas = getHibernateTemplate().loadAll(AutoAttendant.class);
        return aas;
    }

    public AutoAttendant getAutoAttendant(Integer id) {
        return (AutoAttendant) getHibernateTemplate().load(AutoAttendant.class, id);
    }

    public AutoAttendant getAutoAttendantBySystemName(String systemId) {
        Integer id = AutoAttendant.getIdFromSystemId(systemId);
        if (id != null) {
            return getAutoAttendant(id);
        }
        return getAttendant(systemId);
    }

    public void deleteAutoAttendantsByIds(Collection<Integer> attendantIds) {
        for (Integer id : attendantIds) {
            AutoAttendant aa = getAutoAttendant(id);
            deleteAutoAttendant(aa);
        }
        if (!attendantIds.isEmpty()) {
            replicateConfig();
        }
    }

    public void deleteAutoAttendant(AutoAttendant attendant) {
        if (attendant.isPermanent()) {
            throw new AttendantInUseException();
        }

        attendant.setValueStorage(clearUnsavedValueStorage(attendant.getValueStorage()));
        getHibernateTemplate().refresh(attendant);

        Collection<AttendantRule> attendantRules = getHibernateTemplate().loadAll(AttendantRule.class);
        Collection affectedRules = new ArrayList();
        for (AttendantRule rule : attendantRules) {
            if (rule.checkAttendant(attendant)) {
                affectedRules.add(rule);
            }
        }
        if (!affectedRules.isEmpty()) {
            throw new AttendantInUseException(affectedRules);
        }

        // deselect special mode if removign attendant
        AttendantSpecialMode specialMode = loadAttendantSpecialMode();
        if (attendant.equals(specialMode.getAttendant())) {
            specialMode.setAttendant(null);
            specialMode.setEnabled(false);
            getHibernateTemplate().saveOrUpdate(specialMode);
        }

        getHibernateTemplate().delete(attendant);
    }

    public Group getDefaultAutoAttendantGroup() {
        return m_settingDao.getGroupCreateIfNotFound(ATTENDANT_GROUP_ID, "default");
    }

    public AutoAttendant newAutoAttendantWithDefaultGroup() {
        AutoAttendant aa = (AutoAttendant) m_beanFactory.getBean(AutoAttendant.BEAN_NAME, AutoAttendant.class);

        // All auto attendants share same group: default
        Set groups = aa.getGroups();
        if (groups == null || groups.size() == 0) {
            aa.addGroup(getDefaultAutoAttendantGroup());
        }

        return aa;
    }

    public Setting getAttendantSettingModel() {
        AutoAttendant aa = (AutoAttendant) m_beanFactory.getBean(AutoAttendant.BEAN_NAME, AutoAttendant.class);
        return aa.getSettings();
    }

    public AutoAttendant createSystemAttendant(String systemId) {
        AutoAttendant aa = (AutoAttendant) m_beanFactory.getBean(systemId + "Prototype");
        aa.resetToFactoryDefault();
        return aa;
    }

    @Required
    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }

    private Collection getAutoAttendantsWithName(String alias) {
        return getHibernateTemplate().findByNamedQueryAndNamedParam("autoAttendantIdsWithName", "value", alias);
    }

    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection autoAttendants = getAutoAttendantsWithName(alias);
        return BeanId.createBeanIdCollection(autoAttendants, AutoAttendant.class);
    }

    private void replicateConfig() {
        SipxService sipxIvrService = m_sipxServiceManager.getServiceByBeanId(SipxIvrService.BEAN_ID);
        m_serviceConfigurator.replicateServiceConfig(sipxIvrService, true);
    }

    public AutoAttendant createOperator(String attendantId) {
        AutoAttendant attendant = getAttendant(attendantId);
        if (attendant != null) {
            // already exists... - nothing created
            return null;
        }
        attendant = createSystemAttendant(attendantId);
        attendant.addGroup(getDefaultAutoAttendantGroup());
        getHibernateTemplate().saveOrUpdate(attendant);
        return attendant;
    }

    /**
     * This is for testing only.
     */
    public void clear() {
        List attendants = getHibernateTemplate().loadAll(AutoAttendant.class);
        getHibernateTemplate().deleteAll(attendants);
    }

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

    public void selectSpecial(AutoAttendant aa) {
        AttendantSpecialMode specialMode = loadAttendantSpecialMode();
        if (specialMode == null) {
            specialMode = new AttendantSpecialMode();
        }
        specialMode.setAttendant(aa);
        getHibernateTemplate().saveOrUpdate(specialMode);
        if (specialMode.isEnabled()) {
            replicateConfig();
        }
    }

    public void deselectSpecial(AutoAttendant aa) {
        AttendantSpecialMode specialMode = loadAttendantSpecialMode();
        if (specialMode.isEnabled()) {
            LOG.warn("Cannot de select new special attendnat when special attendant active");
            return;
        }

        specialMode.setAttendant(null);
        getHibernateTemplate().saveOrUpdate(specialMode);
        replicateConfig();
    }

    public AutoAttendant getSelectedSpecialAttendant() {
        AttendantSpecialMode specialMode = loadAttendantSpecialMode();
        return specialMode.getAttendant();
    }

    public boolean getSpecialMode() {
        AttendantSpecialMode specialMode = loadAttendantSpecialMode();
        return specialMode.isEnabled();
    }

    public void setSpecialMode(boolean enabled) {
        AttendantSpecialMode specialMode = loadAttendantSpecialMode();
        if (enabled && specialMode.getAttendant() == null) {
            specialMode.setAttendant(getAfterhour());
        }
        specialMode.setEnabled(enabled);
        getHibernateTemplate().saveOrUpdate(specialMode);
        replicateConfig();
    }

    private AttendantSpecialMode loadAttendantSpecialMode() {
        List asm = getHibernateTemplate().loadAll(AttendantSpecialMode.class);
        AttendantSpecialMode specialMode = (AttendantSpecialMode) DataAccessUtils.singleResult(asm);
        return specialMode;
    }

    @Required
    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    @Required
    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }
}
