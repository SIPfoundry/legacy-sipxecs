/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.upload;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FilenameFilter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import org.apache.commons.io.FilenameUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class DefaultSystemFirmwareInstall implements ApplicationListener {
    private static final Log LOG = LogFactory.getLog(DefaultSystemFirmwareInstall.class);

    private static final String UPLOAD_SPEC = "Upload.spec";
    private static final String FILE_SETTING = "File.setting";
    private static final String UPLOAD_FILE = "Upload.file";
    private static final String DEFAULT_ACTIVATION = "ActivateByDefault";
    private static final String OVERRIDE_PREVIOUS_FIRMWARES = "OverridePreviousFirmwares";
    private static final String VERSION = "Version";

    private UploadUtil m_uploadUtil;
    private String m_firmwareDirectory;

    @Required
    public void setUploadUtil(UploadUtil uploadUtil) {
        m_uploadUtil = uploadUtil;
    }

    @Required
    public void setFirmwareDirectory(String firmwareDirectory) {
        m_firmwareDirectory = firmwareDirectory;
    }

    public void installAvailableFirmwares() {
        List<DefaultSystemFirmware> firmwares = findAvailableFirmwares();
        if (null == firmwares || firmwares.isEmpty()) {
            return;
        }

        try {
            ExecutorService executorService = Executors.newFixedThreadPool(firmwares.size());
            for (DefaultSystemFirmware firmware : firmwares) {
                executorService.submit(new InstallAvailableFirmware(firmware));
            }
            executorService.shutdown();
            executorService.awaitTermination(300, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            LOG.error("Unexpected termination of firmware install", e);
        }
    }

    class InstallAvailableFirmware implements Runnable {
        private final DefaultSystemFirmware m_defaultSystemFirmware;

        public InstallAvailableFirmware(DefaultSystemFirmware defaultSystemFirmware) {
            super();
            m_defaultSystemFirmware = defaultSystemFirmware;
        }

        public void run() {
            String[] args = m_defaultSystemFirmware.getUploadArgs();
            if (3 < args.length) {
                String uploadName = "System Default Ver-" + m_defaultSystemFirmware.getVersion();
                Upload upload = m_uploadUtil.addUpload(args, uploadName);
                m_uploadUtil.setUploads(upload, args);

                if (m_defaultSystemFirmware.isActivateByDefault()) {
                    if (m_defaultSystemFirmware.isOverridePreviousFirmwares()) {
                        m_uploadUtil.forceDeploy(upload);
                    } else {
                        m_uploadUtil.deploy(upload);
                    }
                }
            }
        }
    }

    public List<DefaultSystemFirmware> findAvailableFirmwares() {
        File[] firmwareSpecs = getFirmwareSpecFiles();
        if (null == firmwareSpecs) {
            return null;
        }

        List<DefaultSystemFirmware> firmwareList = new ArrayList<DefaultSystemFirmware>();

        for (File firmwareSpec : firmwareSpecs) {
            File markerFile = markerFile(firmwareSpec);
            if (markerFile.exists()) {
                // ignore firmware if we already looked at it
                continue;
            }
            DefaultSystemFirmware defaultSystemFirmware = findFirmware(firmwareSpec);
            if (defaultSystemFirmware != null) {
                firmwareList.add(defaultSystemFirmware);
                try {
                    markerFile.createNewFile();
                } catch (IOException e) {
                    LOG.error("Cannot create a new file in firmware directory.", e);
                }
            }
        }
        return firmwareList;
    }

    public boolean isNewFirmwareAvailable() {
        File[] firmwareSpecs = getFirmwareSpecFiles();
        if (null != firmwareSpecs) {
            for (File firmwareSpec : firmwareSpecs) {
                File markerFile = markerFile(firmwareSpec);
                if (!markerFile.exists()) {
                    return true;
                }
            }
        }
        return false;
    }

    private File[] getFirmwareSpecFiles() {
        File firmwareDirectory = new File(m_firmwareDirectory);

        if (!firmwareDirectory.exists()) {
            return null;
        }

        File[] firmwareSpecs = firmwareDirectory.listFiles(new FilenameFilter() {
            public boolean accept(File dir, String name) {
                return name.endsWith(".fmws");
            }
        });
        return firmwareSpecs;
    }

    private File markerFile(File firmwareSpec) {
        String markerName = "." + FilenameUtils.getBaseName(firmwareSpec.getPath());
        return new File(firmwareSpec.getParent(), markerName);
    }

    private DefaultSystemFirmware findFirmware(File firmwareSpec) {
        BufferedReader input = null;
        try {
            input = new BufferedReader(new FileReader(firmwareSpec));

            String uploadSpec = null;
            List<String> fileSettings = new ArrayList<String>();
            List<String> uploadFiles = new ArrayList<String>();
            String line = null;
            String defaultActivation = null;
            String overridePreviousFirmwares = null;
            String version = null;
            while (null != (line = input.readLine())) {
                String equalsString = "=";

                if (line.startsWith(UPLOAD_SPEC)) {
                    uploadSpec = line.replace(UPLOAD_SPEC + equalsString, StringUtils.EMPTY);
                } else if (line.startsWith(FILE_SETTING)) {
                    fileSettings.add(line.replace(FILE_SETTING + equalsString, StringUtils.EMPTY));
                } else if (line.startsWith(UPLOAD_FILE)) {
                    String uploadFile = line.replace(UPLOAD_FILE + equalsString, StringUtils.EMPTY);
                    uploadFile = m_firmwareDirectory + File.separatorChar + uploadFile;
                    uploadFiles.add(uploadFile);
                } else if (line.startsWith(DEFAULT_ACTIVATION)) {
                    defaultActivation = line.replace(DEFAULT_ACTIVATION + equalsString, StringUtils.EMPTY);
                } else if (line.startsWith(OVERRIDE_PREVIOUS_FIRMWARES)) {
                    overridePreviousFirmwares = line.replace(OVERRIDE_PREVIOUS_FIRMWARES + equalsString,
                            StringUtils.EMPTY);
                } else if (line.startsWith(VERSION)) {
                    version = line.replace(VERSION + equalsString, StringUtils.EMPTY);
                }
            }

            return new DefaultSystemFirmware(uploadSpec, fileSettings, uploadFiles, defaultActivation,
                    overridePreviousFirmwares, version);

        } catch (IOException e) {
            LOG.error("Firmware descriptor errors", e);
        } finally {
            IOUtils.closeQuietly(input);
        }
        return null;
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ApplicationInitializedEvent) {
            installAvailableFirmwares();
        }
    }
}
