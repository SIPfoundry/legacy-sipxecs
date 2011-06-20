/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.cisco;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.device.ProfileFilter;

class BinaryFilter implements ProfileFilter {
    private static final Log LOG = LogFactory.getLog(BinaryFilter.class);

    private static final String TEMP_FILE_PREFIX = "cisco_ata";


    private CiscoModel m_model;

    private String m_systemDirectory;

    public BinaryFilter(String systemDirectory, CiscoModel model) {
        m_systemDirectory = systemDirectory;
        m_model = model;
    }

    public String getPtagDat() {
        return String.format("%s/%s-ptag.dat", m_systemDirectory, m_model.getCfgPrefix());
    }

    /**
     * this points to the cfgfmt utility in etc/cisco directory
     */
    public String getCfgfmtUtility() {
        return m_systemDirectory + "/cfgfmt";
    }

    private void requireFile(String filename) {
        File f = new File(filename);
        if (!f.exists()) {
            String msg = f.getAbsolutePath() + " required: it is supplied by Cisco support.";
            throw new RuntimeException(msg);
        }
    }

    public void copy(InputStream in, OutputStream out) throws IOException {

        File txtFile = File.createTempFile(TEMP_FILE_PREFIX, ".txt");
        FileOutputStream tempOut = new FileOutputStream(txtFile);
        IOUtils.copy(in, tempOut);
        tempOut.close();

        File binFile = File.createTempFile(TEMP_FILE_PREFIX, ".bin");
        requireFile(getCfgfmtUtility());
        requireFile(getPtagDat());
        try {
            String[] cmd = {
                getCfgfmtUtility(), "-t" + getPtagDat(), txtFile.getAbsolutePath(), binFile.getAbsolutePath()
            };
            LOG.info(StringUtils.join(cmd, ' '));
            Process p = Runtime.getRuntime().exec(cmd);
            int errCode = p.waitFor();
            if (errCode != 0) {
                String msg = "Cisco profile conversion failed status code: " + errCode;
                StringWriter err = new StringWriter();
                err.write(msg.toCharArray());
                IOUtils.copy(p.getErrorStream(), err);
                throw new RuntimeException(err.toString());
            }
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }

        FileInputStream binStream = new FileInputStream(binFile);
        IOUtils.copy(binStream, out);
    }
}
