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
import java.util.Collection;
import java.util.Comparator;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.FutureTask;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.beans.factory.annotation.Required;

public class ConfigurationDiagnosticContextImpl implements ConfigurationDiagnosticContext {

    private static final Log LOG = LogFactory.getLog(ConfigurationDiagnosticContextImpl.class);

    private String m_descriptorPath;

    private String m_preflightInstallerLocation;

    private ExternalCommandContext m_externalCommandContext;

    private List<ConfigurationDiagnostic> m_configurationTests;

    private List<Future<Integer>> m_results;

    public synchronized List<ConfigurationDiagnostic> getConfigurationTests() {
        if (m_configurationTests == null) {
            m_configurationTests = loadConfigurationTests();
        }
        return m_configurationTests;
    }

    public synchronized void runTests() {
        if (isInProgress(m_results)) {
            // do not need to start new jobs if old are still running
            return;
        }
        try {
            ExecutorService executor = Executors.newSingleThreadExecutor();
            m_results = scheduleAll(executor, getConfigurationTests());
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Similar to ExecutorService.invokeAll but this method is not going to block. You can check
     * the progress later by analysing returned futures.
     */
    static <T> List<Future<T>> scheduleAll(Executor executor,
            Collection< ? extends Callable<T>> tasks) throws InterruptedException {
        List<Future<T>> futures = new ArrayList<Future<T>>(tasks.size());
        for (Callable<T> t : tasks) {
            FutureTask<T> f = new FutureTask<T>(t);
            futures.add(f);
            executor.execute(f);
        }
        return futures;
    }

    /**
     * Checks if any tests are currently in progress
     *
     * @param results list of test results - can be null
     * @return true is at least one future is not completed yet
     */
    static <T> boolean isInProgress(List<Future<T>> results) {
        if (results == null) {
            return false;
        }
        for (Future<T> future : results) {
            if (!future.isDone()) {
                return true;
            }
        }
        return false;
    }

    /**
     * Loads test descriptors from XML test descriptor files.
     *
     * @return list of test descriptors in the order tests should be run
     */
    private List<ConfigurationDiagnostic> loadConfigurationTests() {
        File[] files = getDescriptorFiles(m_descriptorPath);
        List<ConfigurationDiagnostic> tests = new ArrayList<ConfigurationDiagnostic>(files.length);
        for (File file : files) {
            LOG.debug("Found descriptor file: " + file);
            try {
                ConfigurationDiagnostic diag = new ConfigurationDiagnostic();
                diag.loadFromXml(new FileInputStream(file), m_externalCommandContext);
                tests.add(diag);
            } catch (Exception ioe) {
                LOG.error("Error loading test descriptor file: " + file, ioe);
            }
        }
        return tests;
    }

    /**
     * Finds all test descriptor files
     *
     * @param directory in which descriptor files are located
     * @return list of test descriptor files in the alphabetic order.
     */
    private File[] getDescriptorFiles(String descriptorPath) {
        File descriptorDir = new File(descriptorPath);
        LOG.debug("descriptorPath: " + descriptorPath);
        FilenameFilter descriptorFilter = new FilenameFilter() {
            public boolean accept(File dir, String name) {
                return name.endsWith(".test.xml");
            }
        };
        File[] descriptorFiles = descriptorDir.listFiles(descriptorFilter);
        Arrays.sort(descriptorFiles, new Comparator<File>() {
            public int compare(File o1, File o2) {
                return o1.getName().compareTo(o2.getName());
            }
        });
        return descriptorFiles;
    }

    @Required
    public void setDescriptorPath(String path) {
        m_descriptorPath = path;
    }

    @Required
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
