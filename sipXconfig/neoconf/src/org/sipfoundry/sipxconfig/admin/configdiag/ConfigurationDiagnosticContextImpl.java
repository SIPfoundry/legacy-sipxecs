/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.configdiag;

import java.io.File;
import java.io.FileInputStream;
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.beans.factory.annotation.Required;

public class ConfigurationDiagnosticContextImpl implements ConfigurationDiagnosticContext {

    private static final Log LOG = LogFactory.getLog(ConfigurationDiagnosticContextImpl.class);

    private String m_descriptorPath;

    private String m_preflightInstallerLocation;

    private ExternalCommandContext m_externalCommandContext;

    public List<ConfigurationDiagnostic> getConfigurationTests() {
        List<ConfigurationDiagnostic> configurationTests = new ArrayList<ConfigurationDiagnostic>();

        File descriptorDir = new File(m_descriptorPath);
        LOG.debug("descriptorPath: " + m_descriptorPath);
        LinkedList<File> descriptorFiles = new LinkedList<File>(Arrays
                .<File> asList(descriptorDir.listFiles(new FilenameFilter() {
                    public boolean accept(File dir, String name) {
                        return name.endsWith(".test.xml");
                    }
                })));

        Collections.<File> sort(descriptorFiles, new Comparator<File>() {
            public int compare(File o1, File o2) {
                return o1.getName().compareTo(o2.getName());
            }
        });

        for (File file : descriptorFiles) {
            LOG.debug("Found descriptor file " + file.getName());
            ConfigurationDiagnostic diag = new ConfigurationDiagnostic();
            diag.setCommandContext(m_externalCommandContext);
            try {
                diag.loadFromXml(new FileInputStream(file));
                configurationTests.add(diag);
            } catch (Exception ioe) {
                LOG.error("Error loading test descriptor file " + file.getName(), ioe);
            }
        }

        return configurationTests;
    }

    public void setDescriptorPath(String path) {
        m_descriptorPath = path;
    }

    public void setExternalCommandContext(ExternalCommandContext externalCommandContext) {
        m_externalCommandContext = externalCommandContext;
    }

    @Required
    public void setPreflightInstallerLocation(String preflightInstallerLocation) {
        m_preflightInstallerLocation = preflightInstallerLocation;
    }

    public File getPreflightInstaller() {
        return new File(m_preflightInstallerLocation);
    }
}
