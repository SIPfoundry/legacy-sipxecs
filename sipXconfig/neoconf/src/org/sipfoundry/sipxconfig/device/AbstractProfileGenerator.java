/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.OutputStream;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;

public abstract class AbstractProfileGenerator implements ProfileGenerator {
    private ProfileLocation m_profileLocation;
    private String m_templateRoot;

    public void setProfileLocation(ProfileLocation profileLocation) {
        m_profileLocation = profileLocation;
    }

    public void generate(ProfileContext context, String outputFileName) {

        generate(context, null, outputFileName);
    }

    public void setTemplateRoot(String templateRoot) {
        m_templateRoot = templateRoot;
    }

    public void copy(String inputFileName, String outputFileName) {
        if (outputFileName == null) {
            return;
        }
        OutputStream output = m_profileLocation.getOutput(outputFileName);
        FileInputStream input = null;
        try {
            File sourceFile = new File(m_templateRoot, inputFileName);
            input = new FileInputStream(sourceFile);
            IOUtils.copy(input, output);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(input);
            IOUtils.closeQuietly(output);
        }
    }

    public void generate(ProfileContext context, ProfileFilter filter, String outputFileName) {
        if (outputFileName == null) {
            return;
        }

        OutputStream wtr = m_profileLocation.getOutput(outputFileName);
        try {
            if (filter == null) {
                generateProfile(context, wtr);
            } else {
                ByteArrayOutputStream unformatted = new ByteArrayOutputStream();
                generateProfile(context, unformatted);
                unformatted.close();
                filter.copy(new ByteArrayInputStream(unformatted.toByteArray()), wtr);
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(wtr);
        }
    }

    public void remove(String outputFileName) {
        if (StringUtils.isNotEmpty(outputFileName)) {
            m_profileLocation.removeProfile(outputFileName);
        }
    }

    protected abstract void generateProfile(ProfileContext context, OutputStream out) throws IOException;
}
