/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

/**
 * Logout copied from Vlib example, also see
 * http://thread.gmane.org/gmane.comp.java.tapestry.user/31641
 */
public interface ApplicationLifecycle {
    /**
     * Logs the user out of the systems; sets a flag that causes the session to be discarded at the
     * end of the request.
     */
    void logout();

    /**
     * If true, then the session (if it exists) should be discarded at the end of the request.
     */
    boolean getDiscardSession();
}
