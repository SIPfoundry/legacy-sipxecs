/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.tls;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.dbunit.dataset.ITable;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.permission.PermissionName;

public class TlsPeerManagerImplTestIntegration extends IntegrationTestCase {

    private static final int NUM_BRANCHES = 5;

    private TlsPeerManager m_tlsPeerManager;

    public void testDeleteTlsPeer() throws Exception {
        loadDataSet("admin/tls/tls_peer.db.xml");

        assertEquals(2, m_tlsPeerManager.getTlsPeers().size());

        TlsPeer peer = m_tlsPeerManager.getTlsPeer(1);
        assertNotNull(peer);
        m_tlsPeerManager.deleteTlsPeer(peer);

        assertEquals(1, m_tlsPeerManager.getTlsPeers().size());
    }

    public void testDeleteTlsPeers() throws Exception {
        loadDataSet("admin/tls/tls_peer.db.xml");

        assertEquals(2, m_tlsPeerManager.getTlsPeers().size());
        assertNotNull(m_tlsPeerManager.getTlsPeerByName("tlspeer1"));

        Collection<Integer> ids = new ArrayList<Integer>();
        ids.add(1);
        ids.add(2);

        m_tlsPeerManager.deleteTlsPeers(ids);

        assertEquals(0, m_tlsPeerManager.getTlsPeers().size());
    }

    public void testNewTlsPeer() throws Exception {
        TlsPeer peer = m_tlsPeerManager.newTlsPeer();
        assertNotNull(peer.getInternalUser().getSipPassword());
        assertEquals(false, peer.getInternalUser().getSettingTypedValue(PermissionName.VOICEMAIL.getPath()));
        assertEquals(false, peer.getInternalUser().getSettingTypedValue(PermissionName.FREESWITH_VOICEMAIL.getPath()));
    }

    public void saveTlsPeer() throws Exception {
        loadDataSet("admin/tls/tls_peer.db.xml");

        assertEquals(2, m_tlsPeerManager.getTlsPeers().size());

        TlsPeer peer = m_tlsPeerManager.newTlsPeer();
        peer.setName("tlspeer3");
        m_tlsPeerManager.saveTlsPeer(peer);

        assertEquals(3, m_tlsPeerManager.getTlsPeers().size());
        assertEquals("~~tp~tlspeer3", peer.getInternalUser().getUserName());

        TlsPeer peer1 = m_tlsPeerManager.getTlsPeerByName("tlspeer1");
        peer1.setName(" tl spe e r4");
        m_tlsPeerManager.saveTlsPeer(peer1);
        assertEquals("~~tp~tlspeer4", peer1.getInternalUser().getUserName());

        TlsPeer peer2 = m_tlsPeerManager.getTlsPeerByName("tlspeer2");
        peer2.setName("tlspeer4");
        try {
            m_tlsPeerManager.saveTlsPeer(peer2);
            fail();
        } catch (UserException ex) {

        }
    }

    public void setTlsPeerManager(TlsPeerManager peerManager) {
        m_tlsPeerManager = peerManager;
    }

}
