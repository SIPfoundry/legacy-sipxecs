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
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.dao.support.DataAccessUtils;

public class LocalizationContextImpl extends SipxHibernateDaoSupport<Localization> implements LocalizationContext,
        ApplicationContextAware {

    public static final String PROMPTS_PREFIX = "stdprompts_";
    private static final Log LOG = LogFactory.getLog(LocalizationContextImpl.class);
    private String m_promptsDir;
    private String m_defaultRegion;
    private String m_defaultLanguage;
    private ApplicationContext m_applicationContext;

    public void setPromptsDir(String promptsDir) {
        m_promptsDir = promptsDir;
    }

    public void setDefaultRegion(String defaultRegion) {
        m_defaultRegion = defaultRegion;
    }

    public void setDefaultLanguage(String defaultLanguage) {
        m_defaultLanguage = defaultLanguage;
        // Calling getLocalization() populates the localization table
        // when empty
        getLocalization();
    }

    @Override
    public String getCurrentRegionId() {
        return getLocalization().getRegionId();
    }

    @Override
    public String getCurrentLanguage() {
        return getLocalization().getLanguage();
    }

    @Override
    public String getCurrentLanguageDir() {
        String language = getLocalization().getLanguage();
        if (StringUtils.equals(language, DEFAULT)) {
            return PROMPTS_DEFAULT;
        }
        return PROMPTS_PREFIX + language;
    }

    @Override
    public String[] getInstalledLanguages() {
        String[] languageDirs = getListOfDirectories(m_promptsDir, PROMPTS_PREFIX);
        String[] languages = new String[languageDirs.length];
        for (int i = 0; i < languageDirs.length; i++) {
            String languageDir = languageDirs[i];
            languages[i] = languageDir.substring(PROMPTS_PREFIX.length());
        }

        return languages;
    }

    @Override
    public String[] getInstalledLanguageDirectories() {
        return getListOfDirectories(m_promptsDir, PROMPTS_PREFIX);
    }

    protected String[] getListOfDirectories(String path, String prefix) {
        return new File(path).list(new PrefixFileFilter(prefix));
    }

    @Override
    public Localization getLocalization() {
        List<Localization> l = getHibernateTemplate().loadAll(Localization.class);
        Localization localization = DataAccessUtils.singleResult(l);
        if (localization == null) {
            // The localization table is empty - create a new localization using
            // default values and update the table
            localization = new Localization();
            localization.setRegion(m_defaultRegion);
            localization.setLanguage(m_defaultLanguage);
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
    @Override
    public void updateRegion(String regionBeanId) {
        Localization localization = getLocalization();
        if (localization.getRegion().equals(regionBeanId)) {
            return;
        }

        localization.setRegion(regionBeanId);
        m_applicationContext.publishEvent(new RegionUpdatedEvent(this, regionBeanId));
        getHibernateTemplate().saveOrUpdate(localization);
        getHibernateTemplate().flush();
        getDaoEventPublisher().publishSave(localization);
    }

    /**
     * Set new current language
     *
     * @return positive value is success, negative if failure, 0 if there was no change
     */
    @Override
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
        getHibernateTemplate().saveOrUpdate(localization);
        getHibernateTemplate().flush();
        // TODO: do we really need this? It does not seem to be caught anywhere!
        getDaoEventPublisher().publishSave(localization);
        // Copy default AutoAttendant prompts in the currently applied language
        // to AutoAttendant prompts directory.
        LOG.debug("Language updated, sending LanguageUpdatedEvent...");
        m_applicationContext.publishEvent(new LanguageUpdatedEvent(this, m_promptsDir, getCurrentLanguageDir()));
        return 1;
    }

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }
}
