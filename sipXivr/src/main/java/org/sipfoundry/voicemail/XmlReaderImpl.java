/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.voicemail;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;

import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.io.SAXReader;

/**
 * Helper class for reading/writing XML
 */
public abstract class XmlReaderImpl<T> {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    
    public T readObject(File file) {
        Reader ioreader = null;
        T object = null;
        try {
            if (file.exists()) {
                ioreader = new FileReader(file);
                object = readObject(ioreader); 
            }        
            return object;
        } catch (IOException e) {
            LOG.error("Cannot read from file "+file.getAbsolutePath());
            throw new RuntimeException(e);
        } finally {
           IOUtils.closeQuietly(ioreader);
        }                
    }
    
    public T readObject(Reader input) {
        SAXReader reader = new SAXReader();
        Document doc;
        try {
            doc = reader.read(input);
        } catch (DocumentException e) {
            throw new XmlFormatError("Parsing mailbox preferences", e);
        }
        return readObject(doc);
    }
    
    public static class XmlFormatError extends RuntimeException {
        /**
         * 
         */
        private static final long serialVersionUID = -4147912428127676776L;

        public XmlFormatError(String message, Exception cause) {
            super(message, cause);
        }
    }
    
    public abstract T readObject(Document doc);
}
