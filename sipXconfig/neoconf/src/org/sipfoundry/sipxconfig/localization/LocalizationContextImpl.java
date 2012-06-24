/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.localization;


import java.io.File;
import java.util.List;

import org.apache.commons.io.filefilter.PrefixFileFilter;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.commserver.imdb.ReplicationManager;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.dialplan.DialPlanSetup;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;

public class LocalizationContextImpl extends SipxHibernateDaoSupport implements LocalizationContext {
    private static final String PROMPTS_PREFIX = "stdprompts_";
    private String m_promptsDir;
    private String m_defaultRegion;
    private String m_defaultLanguage;
    private DialPlanSetup m_dialplanSetup;
    private AutoAttendantManager m_autoAttendantManager;
    private ReplicationManager m_replicationManager;

    public void setPromptsDir(String promptsDir) {
        m_promptsDir = promptsDir;
    }

    public void setDefaultRegion(String defaultRegion) {
        m_defaultRegion = defaultRegion;
    }

    @Required
    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }

    public void setDefaultLanguage(String defaultLanguage) {
        m_defaultLanguage = defaultLanguage;
        // Calling getLocalization() populates the localization table
        // when empty
        getLocalization();
    }

    @Required
    public void setDialPlanSetup(DialPlanSetup resetDialPlanTask) {
        m_dialplanSetup = resetDialPlanTask;
    }

    public String getCurrentRegionId() {
        return getLocalization().getRegionId();
    }

    public String getCurrentLanguage() {
        return getLocalization().getLanguage();
    }

    public String getCurrentLanguageDir() {
        String language = getLocalization().getLanguage();
        if (StringUtils.equals(language, DEFAULT)) {
            return PROMPTS_DEFAULT;
        }
        return PROMPTS_PREFIX + language;
    }

    public String[] getInstalledLanguages() {
        String[] languageDirs = getListOfDirectories(m_promptsDir, PROMPTS_PREFIX);
        String[] languages = new String[languageDirs.length];
        for (int i = 0; i < languageDirs.length; i++) {
            String languageDir = languageDirs[i];
            languages[i] = languageDir.substring(PROMPTS_PREFIX.length());
        }

        return languages;
    }

    public String[] getInstalledLanguageDirectories() {
        return getListOfDirectories(m_promptsDir, PROMPTS_PREFIX);
    }

    protected String[] getListOfDirectories(String path, String prefix) {
        return new File(path).list(new PrefixFileFilter(prefix));
    }

    public Localization getLocalization() {
        List l = getHibernateTemplate().loadAll(Localization.class);
        Localization localization = (Localization) DataAccessUtils.singleResult(l);
        if (localization == null) {
            // The localization table is empty - create a new localization using
            // default values and update the table
            localization = new Localization();
            localization.setRegion(m_defaultRegion);
            localization.setLanguage(m_defaultLanguage);
            getDaoEventPublisher().publishSave(localization);
            getHibernateTemplate().saveOrUpdate(localization);
        }
        return localization;
    }

    /**
     * Set new current region
     *
     * @return true if there was a change , false if no change was neccessary
     * @throw exception if not successful
     */
    public void updateRegion(String regionBeanId) {
        Localization localization = getLocalization();
        if (localization.getRegion().equals(regionBeanId)) {
            return;
        }

        localization.setRegion(regionBeanId);
        m_dialplanSetup.setup(regionBeanId);
        getHibernateTemplate().saveOrUpdate(localization);
        getHibernateTemplate().flush();
        getDaoEventPublisher().publishSave(localization);
    }

    /**
     * Set new current language
     *
     * @return positive value is success, negative if failure, 0 if there was no change
     */
    public int updateLanguage(String language) {
        if (language == null) {
            return -1;
        }
        Localization localization = getLocalization();
        if (localization.getLanguage().equals(language)) {
            // no change
            return 0;
        }
        // The language has been changed - handle the change
        localization.setLanguage(language);
        // Copy default AutoAttendant prompts in the currently applied language
        // to AutoAttendant prompts directory.
        m_autoAttendantManager.updatePrompts(new File(m_promptsDir, getCurrentLanguageDir()));
        getHibernateTemplate().saveOrUpdate(localization);
        getHibernateTemplate().flush();
        getDaoEventPublisher().publishSave(localization);
        m_replicationManager.replicateAllData(DataSet.MAILSTORE);
        return 1;
    }

    @Required
    public void setReplicationManager(ReplicationManager replManager) {
        m_replicationManager = replManager;
    }
}
