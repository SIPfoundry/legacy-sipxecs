/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.localization;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.io.filefilter.PrefixFileFilter;
import org.apache.commons.io.filefilter.SuffixFileFilter;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContextImpl.RegionDialPlanException;
import org.sipfoundry.sipxconfig.admin.dialplan.ResetDialPlanTask;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;

public class LocalizationContextImpl extends SipxHibernateDaoSupport implements LocalizationContext {

    private static final Log LOG = LogFactory.getLog(LocalizationContextImpl.class);

    private static final String REGION_PREFIX = "region_";
    private static final String PROMPTS_PREFIX = "stdprompts_";
    private static final String DIALPLAN = ".dialPlan";
    private static final String DIALPLAN_TEMPLATE = "dialrules.beans.xml";

    private String m_regionDir;
    private String m_promptsDir;
    private String m_binDir;
    private String m_thirdPartyDir;
    private String m_javaDir;
    private String m_defaultRegion;
    private String m_defaultLanguage;
    private ResetDialPlanTask m_resetDialPlanTask;
    private ServiceConfigurator m_serviceConfigurator;
    private DialPlanActivationManager m_dialPlanActivationManager;
    private AutoAttendantManager m_autoAttendantManager;
    private ConferenceBridgeContext m_conferenceBridgeContext;

    public void setRegionDir(String regionDir) {
        m_regionDir = regionDir;
    }

    public void setPromptsDir(String promptsDir) {
        m_promptsDir = promptsDir;
    }

    public void setBinDir(String binDir) {
        m_binDir = binDir;
    }

    public void setThirdPartyDir(String thirdPartyDir) {
        m_thirdPartyDir = thirdPartyDir;
    }

    public void setDefaultRegion(String defaultRegion) {
        m_defaultRegion = defaultRegion;
    }

    @Required
    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }

    @Required
    public void setDialPlanActivationManager(DialPlanActivationManager dialPlanActivationManager) {
        m_dialPlanActivationManager = dialPlanActivationManager;
    }

    @Required
    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }

    @Required
    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    public void setDefaultLanguage(String defaultLanguage) {
        m_defaultLanguage = defaultLanguage;
        // Calling getLocalization() populates the localization table
        // when empty
        getLocalization();
    }

    @Required
    public void setResetDialPlanTask(ResetDialPlanTask resetDialPlanTask) {
        m_resetDialPlanTask = resetDialPlanTask;
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

    public String[] getInstalledRegions() {
        String[] regions = getListOfDirectories(m_regionDir, REGION_PREFIX);
        List<String> result = new ArrayList<String>(regions.length);
        for (String region : regions) {
            File regionDir = new File(m_regionDir, region);
            String[] files = regionDir.list(new SuffixFileFilter(DIALPLAN_TEMPLATE));
            if (files != null && files.length > 0) {
                result.add(region);
            }
        }
        return result.toArray(new String[result.size()]);
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
            getHibernateTemplate().saveOrUpdate(localization);
        }
        return localization;
    }

    /**
     * Set new current region
     *
     * @return positive value is success, negative if failure, 0 if there was no change
     */
    public int updateRegion(String region) {
        Localization localization = getLocalization();
        if (localization.getRegion().equals(region)) {
            // no change
            return 0;
        }

        // The region has been changed - handle the change
        localization.setRegion(region);
        String regionId = localization.getRegionId();
        if (regionId == null) {
            return -1;
        }
        try {
            String dialPlanBeanId = regionId + DIALPLAN;
            m_resetDialPlanTask.reset(dialPlanBeanId);
            m_dialPlanActivationManager.replicateDialPlan(false);
            getHibernateTemplate().saveOrUpdate(localization);
        } catch (RegionDialPlanException e) {
            LOG.error("Trying to set unsupported region: " + region);
            return -1;
        }

        return 1;
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
        // new to push domain config and dial plans for all locations...
        m_serviceConfigurator.initLocations();
        m_conferenceBridgeContext.updateConfAudio();
        return 1;
    }

    /**
     * Copy uploaded package and run 'sipxlocalization' to install it.
     */
    public void installLocalizationPackage(InputStream stream, String name) {
        File localizationPackage = createLocalizationPackage(stream, name);
        applyLocalizationPackage(localizationPackage);
    }

    private File createLocalizationPackage(InputStream stream, String name) {
        File uploadDirectory = new File(m_regionDir, "localization_packages");
        File fileToApply = new File(uploadDirectory.getPath(), name);
        OutputStream os = null;
        try {
            os = FileUtils.openOutputStream(fileToApply);
            IOUtils.copy(stream, os);
        } catch (IOException ex) {
            LOG.warn("Cannot upload file", ex);
            throw new UserException("message.upload.failed");
        } finally {
            IOUtils.closeQuietly(os);
        }
        return fileToApply;
    }

    private void applyLocalizationPackage(File fileToApply) {
        UserException installFailureException = new UserException("message.installError");
        try {
            String[] cmd = new String[] {
                m_binDir + File.separator + "sipxlocalization", fileToApply.getPath(), m_promptsDir, m_regionDir,
                m_thirdPartyDir, m_javaDir
            };
            Process p = Runtime.getRuntime().exec(cmd);
            BufferedReader scriptErrorReader = new BufferedReader(new InputStreamReader(p.getErrorStream()));
            String errorLine = scriptErrorReader.readLine();
            while (errorLine != null) {
                LOG.warn("sipxlocalization: " + errorLine);
                errorLine = scriptErrorReader.readLine();
            }

            p.waitFor();
            if (p.exitValue() != 0) {
                throw installFailureException;
            }
        } catch (InterruptedException ex) {
            LOG.error("Interrupted when waiting for sipxlocalization script.", ex);
            throw installFailureException;
        } catch (IOException ex) {
            LOG.error("Problems with executing sipxlocalization script.", ex);
            throw installFailureException;
        }
    }

    public void setJavaDir(String javaDir) {
        this.m_javaDir = javaDir;
    }

    public String getJavaDir() {
        return m_javaDir;
    }
}
