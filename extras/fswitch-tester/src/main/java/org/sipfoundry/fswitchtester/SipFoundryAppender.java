/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.fswitchtester;

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

    long nextTimeCheck = 0L;

    public void append(LoggingEvent event) {
        long now = System.currentTimeMillis();
        if (now > nextTimeCheck) {
            nextTimeCheck = now + 15000; // Check again in 15 seconds
            closeFile(); // Close current file.
            activateOptions(); // Reopen it
        }
        super.append(event);
    }

    public SipFoundryAppender(Layout layout, String file) throws IOException {
        super(layout, file);
    }
}
