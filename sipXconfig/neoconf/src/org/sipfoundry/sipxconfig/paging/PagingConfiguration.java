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
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringWriter;
import java.io.Writer;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigurationFile;

public class PagingConfiguration implements ConfigurationFile {
    private static final Log LOG = LogFactory.getLog(PagingConfiguration.class);

    private static final String LOG_LEVEL = "log.level";

    private Map<String, String> m_configProperties;

    public void generate(List<PagingGroup> pagingGroups, String audioDirectory, String domain) {
        // FIXME: hardcoded notice log level
        m_configProperties = generateConfigProperties(pagingGroups, "NOTICE", audioDirectory,
                domain);
    }

    Map<String, String> generateConfigProperties(List<PagingGroup> pagingGroups,
            String logLevelValue, String audioDirectory, String domain) {
        Map<String, String> configPageProperties = new LinkedHashMap<String, String>();

        final String sipPort = "${PAGE_SERVER_SIP_PORT}";

        configPageProperties.put(LOG_LEVEL, logLevelValue);
        configPageProperties.put("log.file", "${PAGE_LOG_DIR}/sipxpage.log");
        configPageProperties.put("sip.address", "${PAGE_SERVER_ADDR}");
        configPageProperties.put("sip.udpPort", sipPort);
        configPageProperties.put("sip.tcpPort", sipPort);
        configPageProperties.put("sip.tlsPort", "${PAGE_SERVER_SIP_SECURE_PORT}");
        configPageProperties.put("rtp.port", "8500");

        int pagingGroupIndex = 1;

        for (PagingGroup group : pagingGroups) {
            if (group.isEnabled()) {
                group.addProperties(configPageProperties, pagingGroupIndex, audioDirectory,
                        domain);
                pagingGroupIndex++;
            }
        }

        return configPageProperties;
    }

    public ConfigFileType getType() {
        return ConfigFileType.PAGING_CONFIG;
    }

    public void write(Writer writer) throws IOException {
        // TODO: Replace with Velocity

        // ConfigFileWriter configWriter = new ConfigFileWriter(configFile);
        // configWriter.reset();
        // configWriter.store(m_configProperties);
    }

    public void writeToFile(File configDir, String filename) throws IOException {
        File outputFile = new File(configDir, filename);
        FileWriter writer = new FileWriter(outputFile);
        write(writer);
    }

    public String getFileContent() {
        try {
            StringWriter out = new StringWriter();
            write(out);
            return out.toString();
        } catch (IOException e) {
            LOG.error("Rethrowing unexpected: " + e.getMessage());
            throw new RuntimeException(e);
        }

    }
}
