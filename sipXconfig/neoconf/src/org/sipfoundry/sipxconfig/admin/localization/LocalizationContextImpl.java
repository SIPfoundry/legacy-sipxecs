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

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.io.filefilter.PrefixFileFilter;
import org.apache.commons.io.filefilter.SuffixFileFilter;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.NoSuchBeanDefinitionException;
import org.springframework.dao.support.DataAccessUtils;

public class LocalizationContextImpl extends SipxHibernateDaoSupport implements
        LocalizationContext {

    private static final Log LOG = LogFactory.getLog(LocalizationContextImpl.class);

    private static final String REGION_PREFIX = "region_";
    private static final String PROMPTS_PREFIX = "stdprompts_";
    private static final String DIALPLAN = ".dialPlan";
    private static final String DIALPLAN_TEMPLATE = "dialrules.beans.xml";

    private String m_regionDir;
    private String m_promptsDir;
    private String m_binDir;
    private String m_defaultRegion;
    private String m_defaultLanguage;
    private DialPlanContext m_dialPlanContext;

    public void setRegionDir(String regionDir) {
        m_regionDir = regionDir;
    }

    public void setPromptsDir(String promptsDir) {
        m_promptsDir = promptsDir;
    }

    public void setBinDir(String binDir) {
        m_binDir = binDir;
    }

    public void setDefaultRegion(String defaultRegion) {
        m_defaultRegion = defaultRegion;
    }

    public void setDefaultLanguage(String defaultLanguage) {
        m_defaultLanguage = defaultLanguage;
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public String getCurrentRegionId() {
        return getLocalization().getRegionId();
    }

    public String getCurrentLanguageId() {
        return getLocalization().getLanguageId();
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
        return getListOfDirectories(m_promptsDir, PROMPTS_PREFIX);
    }

    private String[] getListOfDirectories(String path, String prefix) {
        return new File(path).list(new PrefixFileFilter(prefix));
    }

    public Localization getLocalization() {
        List l = getHibernateTemplate().loadAll(Localization.class);
        Localization localization = (Localization) DataAccessUtils.uniqueResult(l);
        if (localization != null) {
            return localization;
        }
        localization = new Localization();
        localization.setRegion(m_defaultRegion);
        localization.setLanguage(m_defaultLanguage);
        getHibernateTemplate().save(localization);
        return localization;
    }

    /**
     * Set new current region
     * 
     * @return postive value is success, negative if failure, 0 if there was no change
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
            m_dialPlanContext.resetToFactoryDefault(dialPlanBeanId);
            LOG.warn("resetToFactoryDefault : bean " + dialPlanBeanId);

            getHibernateTemplate().saveOrUpdate(localization);
        } catch (NoSuchBeanDefinitionException e) {
            LOG.error("Trying to set unsupported region: " + region);
            return -1;
        }

        return 1;
    }

    /**
     * Set new current language
     * 
     * @return postive value is success, negative if failure, 0 if there was no change
     */
    public int updateLanguage(String language) {
        Localization localization = getLocalization();
        if (localization.getLanguage().equals(language)) {
            // no change
            return 0;
        }

        // The language has been changed - handle the change
        // TODO: add code that sets the language
        localization.setLanguage(language);
        getHibernateTemplate().saveOrUpdate(localization);
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
                m_binDir + File.pathSeparator + "sipxlocalization", fileToApply.getPath(),
                m_promptsDir, m_regionDir
            };
            Process p = Runtime.getRuntime().exec(cmd);
            p.waitFor();
        } catch (InterruptedException ex) {
            LOG.error("Interrupted when waiting for sipxlocalization script.", ex);
            throw installFailureException;
        } catch (IOException ex) {
            LOG.error("Problems with executing sipxlocalization script.", ex);
            throw installFailureException;
        }
    }
}
