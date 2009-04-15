/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.service.SipxService;

/**
 * To be used as a memo containing the true location status.
 *
 * If running service status corresponds to what sipXconfig knows about the location both list are
 * empty. Otherwise it keeps the lists of services to be started and the list of services to be
 * stopped on a location to make sure that server only runs services that are specified in the
 * location bundle list.
 */
public class LocationStatus {
    private final Collection<SipxService> m_toBeStopped;
    private final Collection<SipxService> m_toBeStarted;

    public LocationStatus(Collection<SipxService> toBeStarted, Collection<SipxService> toBeStopped) {
        m_toBeStarted = toBeStarted;
        m_toBeStopped = toBeStopped;
    }

    public Collection<SipxService> getToBeStarted() {
        return Collections.unmodifiableCollection(m_toBeStarted);
    }

    public Collection<SipxService> getToBeStopped() {
        return m_toBeStopped;
    }
}
