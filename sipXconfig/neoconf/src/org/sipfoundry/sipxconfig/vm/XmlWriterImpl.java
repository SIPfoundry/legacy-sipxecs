/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.vm.MailboxManagerImpl.MailstoreMisconfigured;

/**
 * Helper class for reading/writing XML
 */
public abstract class XmlWriterImpl<T> {
    private String m_template;
    private VelocityEngine m_velocityEngine;

    /**
     * Convience method. This will
     *  - create parent directory if it doesn't exist
     *  - wrap io exception in MailstoreMisconfigured exception
     */
    public void writeObject(T object, File file) {
        Writer iowriter = null;
        try {
            if (!file.getParentFile().exists()) {
                file.getParentFile().mkdirs();
            }
            iowriter = new FileWriter(file);
            writeObject(object, iowriter);
        } catch (IOException e) {
            throw new MailstoreMisconfigured("Cannot write to file ", e);
        } finally {
            IOUtils.closeQuietly(iowriter);
        }
    }

    public String getTemplate() {
        return m_template;
    }

    public void setTemplate(String template) {
        m_template = template;
    }

    public void writeObject(T object, Writer output) {
        VelocityContext velocityContext = new VelocityContext();
        addContext(velocityContext, object);
        writeObject(velocityContext, output);
    }

    protected abstract void addContext(VelocityContext context, T object);

    protected void writeObject(VelocityContext context, Writer output) {
        try {
            getVelocityEngine().mergeTemplate(getTemplate(), context, output);
        } catch (Exception e) {
            throw new RuntimeException("Error using velocity template " + getTemplate(), e);
        }
    }

    public VelocityEngine getVelocityEngine() {
        return m_velocityEngine;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }
}
