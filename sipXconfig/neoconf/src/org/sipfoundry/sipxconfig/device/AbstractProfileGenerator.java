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

public abstract class AbstractProfileGenerator implements ProfileGenerator {
    private String m_templateRoot;

    public void setTemplateRoot(String templateRoot) {
        m_templateRoot = templateRoot;
    }

    public void copy(ProfileLocation location, String inputFileName, String outputFileName) {
        copy(location, m_templateRoot, inputFileName, outputFileName);
    }

    public void copy(ProfileLocation location, String inputDirPath, String inputFileName,
            String outputFileName) {
        if (outputFileName == null) {
            return;
        }
        OutputStream output = location.getOutput(outputFileName);
        FileInputStream input = null;
        try {
            File sourceFile = new File(inputDirPath, inputFileName);
            input = new FileInputStream(sourceFile);
            IOUtils.copy(input, output);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(input);
            location.closeOutput(output);
        }
    }

    public void generate(ProfileLocation location, ProfileContext context, ProfileFilter filter,
            String outputFileName) {
        if (outputFileName == null) {
            return;
        }

        OutputStream wtr = location.getOutput(outputFileName);
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
            location.closeOutput(wtr);
        }
    }

    protected abstract void generateProfile(ProfileContext context, OutputStream out) throws IOException;
}
