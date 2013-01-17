/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.upload;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;

/**
 * Describing the files required to track and manage a vendor's firmware files
 */
public class Upload extends BeanWithSettings {
    private static final Log LOG = LogFactory.getLog(Upload.class);
    private static final String ZIP_TYPE = "application/zip";
    private static final String ERROR_WRONG_TYPE_FILE = "&error.wrongTypeFile";
    private static final String PATH_SEPARATOR = "/";
    private String m_name;
    private String m_description;
    private UploadSpecification m_specification;
    private String m_specificationId;
    private String m_beanId;
    private String m_uploadRootDirectory;
    private String m_destinationDirectory;
    private boolean m_deployed;
    private String m_directoryId;
    private ModelSource<UploadSpecification> m_specificationSource;

    public Upload() {
    }

    protected Upload(String beanId) {
        m_beanId = beanId;
    }

    public Upload(UploadSpecification specification) {
        m_beanId = specification.getBeanId();
        m_specification = specification;
    }

    void setDirectoryId(String directory) {
        m_directoryId = directory;
    }

    String getDirectoryId() {
        return m_directoryId != null ? m_directoryId : getId().toString();
    }

    public boolean isDeployed() {
        return m_deployed;
    }

    /**
     * Should only be called by DB marshalling and subclasses. See deploy and undeploy
     */
    public void setDeployed(boolean deployed) {
        m_deployed = deployed;
    }

    public UploadSpecification getSpecification() {
        if (m_specification != null) {
            return m_specification;
        }
        if (m_specificationId == null) {
            throw new IllegalStateException("Model ID not set");
        }
        if (m_specificationSource == null) {
            throw new IllegalStateException("ModelSource not set");
        }
        m_specification = m_specificationSource.getModel(m_specificationId);
        return m_specification;
    }

    public String getBeanId() {
        return m_beanId;
    }

    /**
     * Internal, do not call this method. Hibnerate property declared update=false, but still
     * required method be defined.
     */
    public void setBeanId(@SuppressWarnings("unused") String illegal_) {
    }

    public String getSpecificationId() {
        return m_specificationId;
    }

    public void setSpecification(UploadSpecification specification) {
        m_specification = specification;
    }

    public void setSpecificationId(String specificationId) {
        m_specificationId = specificationId;
        m_specification = null;
    }

    @Override
    protected Setting loadSettings() {
        String modelFile = getSpecification().getModelFilePath();
        Setting settings = getModelFilesContext().loadModelFile(modelFile);
        // Hack, bean id should be valid
        settings.acceptVisitor(new UploadDirectorySetter());

        return settings;
    }

    private class UploadDirectorySetter extends AbstractSettingVisitor {
        @Override
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting) {
                FileSetting fileType = (FileSetting) type;
                fileType.setDirectory(getUploadDirectory());
            }
        }
    }

    public class FileDeployer extends AbstractSettingVisitor {
        @Override
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (!(type instanceof FileSetting)) {
                return;
            }
            String filename = setting.getValue();
            if (filename == null) {
                return;
            }
            String contentType = ((FileSetting) type).getContentType();
            String moveTo = (((FileSetting) type).getMoveTo() != null) ? (PATH_SEPARATOR + ((FileSetting) type)
                    .getMoveTo()) : StringUtils.EMPTY;
            // since moveTo might be a relative path, we need to make sure destination
            // directory exists
            File file = new File(getDestinationDirectory());
            file.mkdirs();
            if (contentType.equalsIgnoreCase(ZIP_TYPE)) {
                deployZipFile(new File(file, moveTo), new File(getUploadDirectory(), filename), (FileSetting) type);
            } else {
                deployFile(filename, ((FileSetting) type).getRename(), moveTo);
            }
        }
    }

    private class FileUndeployer extends AbstractSettingVisitor {
        @Override
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (!(type instanceof FileSetting)) {
                return;
            }
            String filename = setting.getValue();
            if (filename == null) {
                return;
            }
            String contentType = ((FileSetting) type).getContentType();
            String moveTo = (((FileSetting) type).getMoveTo() != null) ? (PATH_SEPARATOR + ((FileSetting) type)
                    .getMoveTo()) : StringUtils.EMPTY;
            String moveToDir = getDestinationDirectory() + moveTo;
            if (contentType.equalsIgnoreCase(ZIP_TYPE)) {
                undeployZipFile(new File(moveToDir), new File(getUploadDirectory(),
                        filename), (FileSetting) type);
            } else {
                File f = new File(moveToDir, filename);
                f.delete();
                if (((FileSetting) type).getRename() != null) {
                    File legacyFile = new File(moveToDir, ((FileSetting) type).getRename());
                    legacyFile.delete();
                }
            }
        }
    }

    private void deployFile(String file, String toFileName, String moveTo) {
        InputStream from;
        OutputStream to;
        try {
            from = new FileInputStream(new File(getUploadDirectory(), file));
            File destDir = new File(getDestinationDirectory() + moveTo);
            destDir.mkdirs();
            if (toFileName != null) {
                to = new FileOutputStream(new File(destDir, toFileName));
            } else {
                to = new FileOutputStream(new File(destDir, file));
            }
            IOUtils.copy(from, to);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public void setDestinationDirectory(String destinationDirectory) {
        m_destinationDirectory = destinationDirectory;
    }

    public String getDestinationDirectory() {
        return m_destinationDirectory;
    }

    /**
     * delete all files
     */
    public void remove() {
        undeploy();
        File uploadDirectory = new File(getUploadDirectory());
        try {
            FileUtils.deleteDirectory(uploadDirectory);
        } catch (IOException cantDelete) {
            LOG.error("Could not remove uploaded files", cantDelete);
        }
    }

    public void setUploadRootDirectory(String uploadDirectory) {
        m_uploadRootDirectory = uploadDirectory;
    }

    public String getUploadDirectory() {
        return m_uploadRootDirectory + '/' + getDirectoryId();
    }

    public void deploy() {
        getSettings().acceptVisitor(new FileDeployer());
        m_deployed = true;
    }

    public void undeploy() {
        getSettings().acceptVisitor(new FileUndeployer());
        m_deployed = false;
    }

    public ModelSource<UploadSpecification> getUploadSpecificationSource() {
        return m_specificationSource;
    }

    public void setUploadSpecificationSource(ModelSource<UploadSpecification> specificationSource) {
        m_specificationSource = specificationSource;
    }

    /**
     * Uses zip file list and list of files to be deleted
     */
    static void undeployZipFile(File expandedDirectory, File zipFile, FileSetting fs) {
        if (!checkZipFile(zipFile)) {
            throw new UserException(ERROR_WRONG_TYPE_FILE, zipFile.getName());
        }

        if (!zipFile.canRead()) {
            LOG.warn("Undeploying missing or unreadable file: " + zipFile.getPath());
            return;
        }
        try {
            ZipFile zip = new ZipFile(zipFile);
            Enumeration< ? extends ZipEntry> entries = zip.entries();
            while (entries.hasMoreElements()) {
                ZipEntry entry = entries.nextElement();
                if (entry.isDirectory()) {
                    // do not clean up directory, no guarantee we created them
                    continue;
                }
                if (fs.isExcluded(entry.getName())) {
                    continue;
                }
                File victim = new File(expandedDirectory, entry.getName());
                victim.delete();
            }
        } catch (ZipException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Expand zip file into destination directory
     */
    static void deployZipFile(File expandDirectory, File zipFile, FileSetting fs) {
        if (!checkZipFile(zipFile)) {
            throw new UserException(ERROR_WRONG_TYPE_FILE, zipFile.getName());
        }

        try {
            ZipFile zip = new ZipFile(zipFile);
            Enumeration< ? extends ZipEntry> entries = zip.entries();
            while (entries.hasMoreElements()) {
                InputStream in = null;
                OutputStream out = null;
                try {
                    ZipEntry entry = entries.nextElement();
                    if (fs.isExcluded(entry.getName())) {
                        continue;
                    }
                    File file = new File(expandDirectory, entry.getName());
                    expandDirectory.mkdirs();
                    if (entry.isDirectory()) {
                        file.mkdirs();
                    } else {
                        file.getParentFile().mkdirs();
                        in = zip.getInputStream(entry);
                        out = new FileOutputStream(file);
                        IOUtils.copy(in, out);
                    }
                } finally {
                    IOUtils.closeQuietly(in);
                    IOUtils.closeQuietly(out);
                }
            }
            zip.close();
        } catch (ZipException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private static boolean checkZipFile(File file) {
        String fileExt = ".zip";
        if (file != null && !file.getName().endsWith(fileExt)) {
            return false;
        }
        return true;
    }
}
