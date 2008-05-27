/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.sip;

import java.util.TimeZone;

import org.apache.commons.lang.math.RandomUtils;

public abstract class AbstractMessage {
    static final TimeZone GMT = TimeZone.getTimeZone("GMT");

    public AbstractMessage() {
        super();
    }

    public abstract void createAndSend(String proxyHost, int proxyPort);

    protected long generateUniqueId() {
        return RandomUtils.nextLong();
    }
}
