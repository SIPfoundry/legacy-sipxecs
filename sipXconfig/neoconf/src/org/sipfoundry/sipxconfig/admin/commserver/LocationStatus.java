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
import java.util.LinkedHashSet;

import static java.util.Collections.unmodifiableCollection;

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
        return unmodifiableCollection(m_toBeStarted);
    }

    public Collection<SipxService> getToBeStopped() {
        return unmodifiableCollection(m_toBeStopped);
    }

    /**
     * Calculates the set of services that need to be replicated.
     *
     * The resulting collection contains all services that are started and all services that are
     * affected by them. It does not contain any of the services that are stopped (no need to
     * replicate service that is stopped).
     */
    public Collection<SipxService> getToBeReplicated() {
        // using set to remove duplicates, linked version to maintain the ordering
        Collection<SipxService> services = new LinkedHashSet<SipxService>(m_toBeStarted);
        // add all affected services
        for (SipxService service : m_toBeStarted) {
            service.appendAffectedServices(services);
        }
        for (SipxService service : m_toBeStopped) {
            service.appendAffectedServices(services);
        }
        // and then remove services than we are stopping anyway
        services.removeAll(m_toBeStopped);
        return unmodifiableCollection(services);
    }
}
