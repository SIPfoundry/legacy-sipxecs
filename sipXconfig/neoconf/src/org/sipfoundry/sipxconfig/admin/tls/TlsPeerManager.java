/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.tls;

import java.util.Collection;
import java.util.List;

public interface TlsPeerManager {

    TlsPeer getTlsPeer(Integer tlsPeerId);

    void saveTlsPeer(TlsPeer tlsPeer);

    void deleteTlsPeer(TlsPeer tlsPeer);

    void deleteTlsPeers(Collection<Integer> allSelected);

    List<TlsPeer> getTlsPeers();

    List<TlsPeer> getTlsPeers(Collection<Integer> ids);

    TlsPeer getTlsPeerByName(String name);

    TlsPeer newTlsPeer();
}
