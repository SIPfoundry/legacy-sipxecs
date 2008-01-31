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
 * This bean is a waiting listener wrapper.
 * When waiting mechanism is needed please inject this bean and
 * set your listener
 *
 */
public class WaitingListenerWrapper {
    private WaitingListener m_waitingListener;

    public WaitingListener getWaitingListener() {
        return m_waitingListener;
    }

    public void setWaitingListener(WaitingListener waitingListener) {
        m_waitingListener = waitingListener;
    }
}
