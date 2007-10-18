/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Iterator;

import org.apache.commons.digester.Digester;
import org.apache.commons.digester.ObjectCreateRule;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.xml.sax.SAXException;

public class ProcessDefinitionsSipxProcessModel extends SimpleSipxProcessModel {

    private static final Log LOG = LogFactory.getLog(ProcessDefinitionsSipxProcessModel.class);
    private final File m_configDir;

    public ProcessDefinitionsSipxProcessModel(String configDir) {
        // Defer init until m_configDir is initialized
        super(true);
        LOG.debug("constructing with " + configDir);
        m_configDir = new File(configDir);

        try {
            initialize();
        } catch (Exception e) {
            LOG.warn("Initialize threw exception: " + e.getMessage());
            LOG.warn("Using SimpleSipxProcessModel init instead");
            super.initialize();
        }
    }

    protected void initialize() {
        String[] extensions = new String[] {
            "xml"
        };
        Iterator it = FileUtils.iterateFiles(m_configDir, extensions, false);
        for (; it.hasNext();) {
            File file = (File) it.next();
            LOG.debug("Initialize processed file " + file.getPath());
            try {
                processFile(file);
            } catch (RuntimeException e) {
                LOG.error("Failed to parse " + file.getPath() + " Exception: " + e.getMessage());
            }
        }
    }

    private void processFile(File file) {
        FileInputStream is = null;
        String pattern = new String("*/group/process");
        try {
            is = new FileInputStream(file);
            Digester digester = new Digester();

            // setting classloader ensures classes are searched for in this classloader
            // instead of parent's classloader is digister was loaded there.
            digester.setClassLoader(getClass().getClassLoader());

            // Do not validate the XML. It is already being done by sipxpbx startup check
            digester.setValidating(false);
            digester.setNamespaceAware(false);

            ProcessCreateRule rule = new ProcessCreateRule(this);

            // Create an instance of class Process
            digester.addRule(pattern, rule);
            // Set name property based on name attribute
            digester.addSetProperties(pattern);

            try {
                digester.parse(is);
            } catch (SAXException se) {
                throw new RuntimeException("Failed to parse watchdog definitions file "
                        + file.getPath(), se);
            }
        } catch (IOException e) {
            throw new RuntimeException("Cannot read watchdog definitions file " + file.getPath(),
                    e);
        } finally {
            IOUtils.closeQuietly(is);
        }
    }

    private static class ProcessCreateRule extends ObjectCreateRule {

        private ProcessDefinitionsSipxProcessModel m_model;

        public ProcessCreateRule(ProcessDefinitionsSipxProcessModel model) {
            super(Process.class);
            m_model = model;
        }

        public void end() throws java.lang.Exception {
            Object obj = digester.peek();
            if (!(obj instanceof Process)) {
                throw new RuntimeException("end digester stack corrupted");
            }

            Process p = (Process) obj;
            if (p.getName() == null) {
                throw new RuntimeException("end Process name not properly set");
            }
            m_model.addProcess(p);
            super.end();
        }
    }
}
