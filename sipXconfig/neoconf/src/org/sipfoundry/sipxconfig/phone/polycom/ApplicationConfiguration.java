/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.regex.Pattern;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Phone;

/**
 * Velocity model for generating [MAC ADDRESS].cfg, pointer to all other config files. See page 11
 * of Administration guide for more information
 */
public class ApplicationConfiguration extends ProfileContext {
    private List<String> m_staleDirectories = new ArrayList<String>();

    private String m_directory;

    private String m_serialNumber;

    private String m_parentDir;
    
    private DeviceVersion m_version;

    public ApplicationConfiguration(Phone phone, String parentDir) {
        super(phone, "polycom/mac-address.cfg.vm");
        m_serialNumber = phone.getSerialNumber();
        m_parentDir = parentDir;
        m_version = phone.getDeviceVersion();
    }

    public String getSipBinaryFilename() {
        return "sip.ld";
    }
    
    public String getManufacturorSipTemplate() {
        return m_version == PolycomModel.VER_2_0 ? "sip.cfg" : StringUtils.EMPTY;
    }

    public String getManufacturorPhoneTemplate() {
        return m_version == PolycomModel.VER_2_0 ? "phone1.cfg" : StringUtils.EMPTY;
    }

    public String getAppFilename() {
        return m_serialNumber + ".cfg";
    }

    /**
     * Lazily retrieves the next available directory name
     */
    String getDirectory() {
        if (m_directory != null) {
            return m_directory;
        }

        String endpointDir = m_serialNumber;
        m_staleDirectories.clear();
        m_directory = getNextDirectorySequence(m_parentDir, endpointDir, m_staleDirectories);
        return m_directory;
    }

    static String getNextDirectorySequence(String root, String uniqueBase, List<String> stale) {
        File rootDir = new File(root);
        NextDirectoryScanner nextSequence = new NextDirectoryScanner(uniqueBase);
        String[] matches = rootDir.list(nextSequence);
        if (matches != null && matches.length > 0) {
            // mark these for deletion
            stale.addAll(Arrays.asList(matches));
        }

        int seq = nextSequence.getNextSequence();

        String padding = "0000";
        String suffix = padding + Integer.toString(seq);

        // performs a natural modulo at seq = 1000
        int truncateHead = suffix.length() - padding.length();
        return uniqueBase + "." + suffix.substring(truncateHead);
    }

    static class NextDirectoryScanner implements FilenameFilter {

        private Pattern m_basePattern;

        private int m_maxSequence;

        public NextDirectoryScanner(String base) {
            m_basePattern = Pattern.compile(base + "\\.\\d*");
        }

        public boolean accept(File root_, String name) {
            boolean match = m_basePattern.matcher(name).matches();
            if (match) {
                int dot = name.lastIndexOf('.');
                String suffix = name.substring(dot + 1);
                try {
                    int sequence = Integer.parseInt(suffix);
                    m_maxSequence = Math.max(m_maxSequence, sequence);
                } catch (NumberFormatException notARecognizedSuffix) {
                    // should have been caught by regexp
                    notARecognizedSuffix.printStackTrace();
                }
            }
            return match;
        }

        int getNextSequence() {
            return m_maxSequence + 1;
        }
    }

    public String getCoreFilename() {
        return getDirectory() + "/ipmid.cfg";
    }

    public String getSipFilename() {
        return getDirectory() + "/sip.cfg";
    }

    public String getPhoneFilename() {
        return getDirectory() + "/phone.cfg";
    }

    public String getDirectoryFilename() {
        // do not put this in getDirectory() because w/FTP it's not nec. anymore.
        // getDirectory() and respective code should removed
        return m_serialNumber + "-directory.xml";
    }

    public void deleteStaleDirectories() {
        try {
            File tftpRoot = new File(m_parentDir);
            for (String dir : m_staleDirectories) {
                File stale = new File(tftpRoot, dir);
                FileUtils.deleteDirectory(stale);
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
