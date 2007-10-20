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
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;
import java.util.StringTokenizer;
import java.util.Vector;

import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;

/*
import org.springframework.web.context.ContextLoader;
import org.springframework.web.context.WebApplicationContext;
import org.springframework.web.context.support.WebApplicationContextUtils;
*/
public class LocalizationContextImpl extends SipxHibernateDaoSupport implements LocalizationContext {

    private static final Log LOG = LogFactory.getLog(LocalizationContextImpl.class);
    private static final String REGION_PREFIX = "region_";
    private static final String PROMPTS_PREFIX = "stdprompts_";
    private static final String DEFAULT = "default";
    private static final String LANGUAGE_CONFIG = "DefaultLanguage";
    private static final String DIALPLAN = ".dialPlan";
    private static final String DIALPLAN_TEMPLATE = "dialrules.beans.xml";
    private static final String DIRECTORY_SEPARATOR = "/";

    private String m_regionDir;
    private String m_promptsDir;
    private String m_binDir;
    private String m_defaultRegion;
    private String m_defaultLanguage;
    private DialPlanContext m_dialPlanContext;

    public void setRegionDir(String regionDir) {
        m_regionDir = regionDir;
    }

    public String getRegionDir() {
        return m_regionDir;
    }

    public void setPromptsDir(String promptsDir) {
        m_promptsDir = promptsDir;
    }

    public String getPromptsDir() {
        return m_promptsDir;
    }

    public void setBinDir(String binDir) {
        m_binDir = binDir;
    }

    public String getBinDir() {
        return m_binDir;
    }

    public void setDefaultRegion(String defaultRegion) {
        m_defaultRegion = defaultRegion;
    }

    public String getDefaultRegion() {
        return m_defaultRegion;
    }

    public void setDefaultLanguage(String defaultLanguage) {
        m_defaultLanguage = defaultLanguage;
    }

    public String getDefaultLanguage() {
        return m_defaultLanguage;
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public DialPlanContext getDialPlanContext() {
        return m_dialPlanContext;
    }

    public String getCurrentRegionId() {
        return getIdFromString(getLocalization().getRegion());
    }

    public String getCurrentLanguageId() {
        return getIdFromString(getLocalization().getLanguage());
    }

    public String[] getInstalledRegions() {
        Vector regions = getListOfDirecotries(m_regionDir, REGION_PREFIX);
        for (int i = 0; i < regions.size(); i++) {
            File regionDir = new File(m_regionDir + DIRECTORY_SEPARATOR + (String) regions.elementAt(i));
            String[] files = regionDir.list();
            boolean templatefound = false;
            for (int j = 0; j < files.length; j++) {
                if (files[j].contains(DIALPLAN_TEMPLATE)) {
                    templatefound = true;
                } 
            }
            if (!templatefound) {
                regions.removeElementAt(i);
            }
        }         
        String[] result = new String[regions.size()];
        regions.toArray(result);
        return result;
    }

    public String[] getInstalledLanguages() {

        Vector  languages = getListOfDirecotries(m_promptsDir, PROMPTS_PREFIX);
        String[] result = new String[languages.size()];
        languages.toArray(result);
        return result;
    }

    private Vector getListOfDirecotries(String path, String prefix) {
        Vector<String> vector = new Vector<String>();
        File dir = new File(path);
        String[] list = dir.list();

        for (int i = 0; i < list.length; i++) {
            if (list[i].contains(prefix)) {
                vector.addElement((String) list[i]);
            }
        }
        return vector;
    }

    public Localization getLocalization() {
        List list = getHibernateTemplate().loadAll(Localization.class);
        if (!list.isEmpty()) {
            return (Localization) list.get(0);
        }
        Localization localization = new Localization();
        localization.setRegion(m_defaultRegion);
        localization.setLanguage(m_defaultLanguage);
        getHibernateTemplate().save(localization);
        return localization;
    }

    private String getIdFromString(String string) {
        String id = null;
        if (string != null) {
            StringTokenizer st = new StringTokenizer(string, "_");
            while (st.hasMoreTokens()) {
                id = st.nextToken();
            }
        }
        return id;
    }

    public int updateRegion(String region) {
        Localization localization = getLocalization();

        if (region.compareTo(DEFAULT) != 0) {
            if (region.compareTo(localization.getRegion()) != 0) {
                // The region has been changed - handle the change
                String regionId = getIdFromString(region);
                if (regionId != null) {
                    String dialPlanBeanId = regionId + DIALPLAN;
                    getDialPlanContext().resetToFactoryDefault(dialPlanBeanId);
                    LOG.warn("resetToFactoryDefault : bean " + dialPlanBeanId);
                    localization.setRegion(region);
                    getHibernateTemplate().saveOrUpdate(localization);
                    return 1;
                } else {
                    return -1;
                }
            }
        }
        return 0;
    }

    public int updateLanguage(String language) {
        Localization localization = getLocalization();

        if (language.compareTo(DEFAULT) != 0) {
            if (localization.getLanguage().compareTo(language) != 0) {
                // The language has been changed - handle the change
                try {
                    File file = new File(m_promptsDir, LANGUAGE_CONFIG);
                    FileWriter writer = new FileWriter(file);
                    writer.write(language + "\n");
                    writer.close();

                    localization.setLanguage(language);
                    getHibernateTemplate().saveOrUpdate(localization);
                    return 1;
                } catch (IOException e) {
                    return -1;
                }
            }
        }
        return 0;
    }

    public int installLocalizationPackage(InputStream stream, String name) {
        // First store the file in the upload directory
        int result = 1;
        File uploadDirectory = new File(m_regionDir + DIRECTORY_SEPARATOR + "localization_packages");
        if (!uploadDirectory.exists()) {
            uploadDirectory.mkdir();
        }
        File fileToApply = new File(uploadDirectory.getPath(), name);
        OutputStream os = null;
        try {
            os = new FileOutputStream(fileToApply);
            IOUtils.copy(stream, os);
            os.close();
            result = 0;
        } catch (IOException ex) {
            throw new UserException("message.upload.failed");
        } finally {
            IOUtils.closeQuietly(os);
        }
        if (result == 0) {
            // A localization package has been uploaded. Invoke a script that
            // will install the package.
            try {
                Process p = Runtime.getRuntime().exec(
                    m_binDir + DIRECTORY_SEPARATOR + "sipxlocalization "
                    + uploadDirectory.getPath() + DIRECTORY_SEPARATOR + name
                    + " " + m_promptsDir + "  " + m_regionDir);
                result = p.waitFor();
            } catch (InterruptedException ex1) {
                LOG.warn("Exception1: " + ex1);
            } catch (IOException ex2) {
                LOG.warn("Exception2: " + ex2);
            }
        }
        return result;
    }
}

