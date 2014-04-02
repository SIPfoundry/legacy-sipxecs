/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.tls;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.common.ReplicableProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public interface TlsPeerManager extends ReplicableProvider {
    public static final GlobalFeature FEATURE = new GlobalFeature("tlsPeers");

    TlsPeer getTlsPeer(Integer tlsPeerId);

    void saveTlsPeer(TlsPeer tlsPeer);

    void deleteTlsPeer(TlsPeer tlsPeer);

    void deleteTlsPeers(Collection<Integer> allSelected);

    List<TlsPeer> getTlsPeers();

    List<TlsPeer> getTlsPeers(Collection<Integer> ids);

    TlsPeer getTlsPeerByName(String name);

    TlsPeer newTlsPeer();
}
