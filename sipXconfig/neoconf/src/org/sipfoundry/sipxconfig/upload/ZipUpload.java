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

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;

import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;

public class ZipUpload extends Upload {
    private static final Log LOG = LogFactory.getLog(ZipUpload.class);

    @Override
    public void deploy() {
        getSettings().acceptVisitor(new ZipFileDeployer());
        setDeployed(true);
    }

    @Override
    public void undeploy() {
        getSettings().acceptVisitor(new ZipFileUndeployer());
        setDeployed(false);
    }

    private class ZipFileDeployer extends AbstractSettingVisitor {
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting) {
                String filename = setting.getValue();
                if (filename != null) {
                    deployZipFile(new File(getDestinationDirectory()), new File(
                            getUploadDirectory(), filename));
                }
            }
        }
    }

    private class ZipFileUndeployer extends AbstractSettingVisitor {
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting) {
                String filename = setting.getValue();
                if (filename != null) {
                    undeployZipFile(new File(getDestinationDirectory()), new File(
                            getUploadDirectory(), filename));
                }
            }
        }
    }

    /**
     * Uses zip file list and list of files to be deleted
     */
    static void undeployZipFile(File expandedDirectory, File zipFile) {
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
    static void deployZipFile(File expandDirectory, File zipFile) {
        try {
            ZipFile zip = new ZipFile(zipFile);
            Enumeration< ? extends ZipEntry> entries = zip.entries();
            while (entries.hasMoreElements()) {
                InputStream in = null;
                OutputStream out = null;
                try {
                    ZipEntry entry = entries.nextElement();
                    File file = new File(expandDirectory, entry.getName());
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
}
