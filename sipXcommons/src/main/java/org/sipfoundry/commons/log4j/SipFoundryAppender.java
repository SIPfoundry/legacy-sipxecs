/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.log4j;

import java.io.IOException;

import org.apache.log4j.FileAppender;
import org.apache.log4j.Layout;
import org.apache.log4j.spi.LoggingEvent;

/**
 * A log4j FileAppender that closes and reopens the underlying log file every 15
 * seconds. This allows the underlying file to be renamed or deleted, (which is
 * often performed by external rotation mechanisims) and it will be detected and
 * a new file created once a new event is being logged that occurs 15 seconds or
 * more after the previous event.
 *
 * @author Woof!
 *
 */
public class SipFoundryAppender extends FileAppender {

    long m_nextTimeCheck = 0L;

    // Duplicate the parent's constructors
    public SipFoundryAppender() {
        super();
    }

    public SipFoundryAppender(Layout layout, String filename, boolean append) throws IOException {
        super(layout, filename, append);
    }

    public SipFoundryAppender(Layout layout, String filename) throws IOException {
        super(layout, filename);
    }

    public SipFoundryAppender(Layout layout, String filename, boolean append, boolean bufferedIO, int bufferSize) throws IOException {
        super(layout, filename, append, bufferedIO, bufferSize);
    }

    @Override
    public void subAppend(LoggingEvent event) {
        long now = System.currentTimeMillis();
        if (now > m_nextTimeCheck) {
            m_nextTimeCheck = now + 15000; // Check again in 15 seconds
            closeFile(); // Close current file.
            activateOptions(); // Reopen it
        }
        super.subAppend(event);
    }
}
