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
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.io.SAXReader;
import org.sipfoundry.sipxconfig.vm.MailboxManagerImpl.MailstoreMisconfigured;

/**
 * Helper class for reading/writing XML
 */
public abstract class XmlReaderImpl<T> {

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
            throw new MailstoreMisconfigured("Cannot read from file ", e);
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
        public XmlFormatError(String message, Exception cause) {
            super(message, cause);
        }
    }

    public abstract T readObject(Document doc);
}
