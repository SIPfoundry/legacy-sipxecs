/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

/**
 * The object that implements this interface should be set in the Waiting Page
 */
public interface WaitingListener {
    /**
     * This method is called when this is requested by the client (browser) -after it loads the
     * waiting page
     */
    void afterResponseSent();
}
