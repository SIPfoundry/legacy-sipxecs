/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.voicemail;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;

import org.apache.log4j.Logger;

import org.apache.commons.io.IOUtils;
//import org.apache.velocity.VelocityContext;
//import org.apache.velocity.app.VelocityEngine;
//import org.sipfoundry.sipxconfig.vm.MailboxManagerImpl.MailstoreMisconfigured;

/**
 * Helper class for reading/writing XML
 */
public abstract class XmlWriterImpl<T> {  
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    /**
     * Convenience method. This will
     *  - create parent directory if it doesn't exist
     *  - wrap io exception in RuntimeException exception
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
            LOG.error("Cannot write to file "+file.getAbsolutePath());
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(iowriter);
        }
    }

    public abstract void writeObject(T object, Writer output) ;        
    
}
