/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.paging;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;
import java.util.Properties;

import org.apache.commons.io.FileUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigurationFile;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.setting.ConfigFileWriter;

public class PagingConfiguration implements ConfigurationFile {
    public static final String CONTEXT_BEAN_NAME = "pagingConfiguration";

    private static final Log LOG = LogFactory.getLog(PagingConfiguration.class);

    private static final String LOG_LEVEL = "log.level";

    private static final String DESCRIPTION_KEY_FORMAT = "page.group.%d.description";

    private static final String BEEP_KEY_FORMAT = "page.group.%d.beep";

    private static final String USER_KEY_FORMAT = "page.group.%d.user";

    private static final String URLS_KEY_FORMAT = "page.group.%d.urls";

    private static final String FILENAME = "sipxpage.properties.in";

    private DomainManager m_domainManager;

    private String m_pagingConfig;

    private String m_etcDirectory;

    private String m_audioDirectory;

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setEtcDirectory(String etcDirectory) {
        m_etcDirectory = etcDirectory;
    }

    public String getAudioDirectory() {
        return m_audioDirectory;
    }

    public void setAudioDirectory(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }

    public void generate(List<PagingGroup> pagingGroups) {
        File configFile = new File(m_etcDirectory, FILENAME);
        try {
            // preserve value for "log.level" key
            String logLevelValue = "NOTICE";
            if (configFile.exists()) {
                Properties configProperties = new Properties();
                configProperties.load(new FileInputStream(configFile));
                if (configProperties.getProperty(LOG_LEVEL) != null) {
                    logLevelValue = (configProperties.getProperty(LOG_LEVEL));
                }

                ConfigFileWriter configWriter = new ConfigFileWriter(configFile);
                configWriter.reset();
                configWriter.store(generateConfigProperties(pagingGroups, logLevelValue));
                m_pagingConfig = FileUtils.readFileToString(configFile);
            }
        } catch (IOException ex) {
            LOG.error("failed to write sipxpage.properties.in");
        }
    }

    public Properties generateConfigProperties(List<PagingGroup> pagingGroups,
            String logLevelValue) {
        Properties configPageProperties = new Properties();
        String sipPort = "${PAGE_SERVER_SIP_PORT}";
        configPageProperties.setProperty(LOG_LEVEL, logLevelValue);
        configPageProperties.setProperty("log.file", "${PAGE_LOG_DIR}/sipxpage.log");
        configPageProperties.setProperty("sip.address", "${PAGE_SERVER_ADDR}");
        configPageProperties.setProperty("sip.udpPort", sipPort);
        configPageProperties.setProperty("sip.tcpPort", sipPort);
        configPageProperties.setProperty("sip.tlsPort", "${PAGE_SERVER_SIP_SECURE_PORT}");
        configPageProperties.setProperty("rtp.port", "8500");

        int pagingGroupIndex = 1;
        for (PagingGroup group : pagingGroups) {
            if (group.isEnabled()) {
                configPageProperties.setProperty(String.format(DESCRIPTION_KEY_FORMAT,
                        pagingGroupIndex), group.formatDescription());
                configPageProperties.setProperty(
                        String.format(BEEP_KEY_FORMAT, pagingGroupIndex), group
                                .formatBeep(m_audioDirectory));
                configPageProperties.setProperty(
                        String.format(USER_KEY_FORMAT, pagingGroupIndex), group
                                .formatPageGroupNumber());
                configPageProperties.setProperty(
                        String.format(URLS_KEY_FORMAT, pagingGroupIndex), group
                                .formatUrls(m_domainManager.getDomain().getName()));
                pagingGroupIndex++;
            }
        }
        return configPageProperties;
    }

    public String getFileContent() {
        return "file";
    }

    public ConfigFileType getType() {
        return ConfigFileType.PAGING_CONFIG;
    }

    public void write(Writer writer) throws IOException {
        if (m_pagingConfig != null) {
            writer.write(m_pagingConfig);
        }
    }

    public void writeToFile(File configDir, String filename) throws IOException {
        File outputFile = new File(configDir, filename);
        FileWriter writer = new FileWriter(outputFile);
        writer.write(m_pagingConfig);
    }
}
