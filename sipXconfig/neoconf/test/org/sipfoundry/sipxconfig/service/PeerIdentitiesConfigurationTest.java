/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.tls.TlsPeer;
import org.sipfoundry.sipxconfig.admin.tls.TlsPeerManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class PeerIdentitiesConfigurationTest extends TestCase {

    public void testXmlGeneration() throws Exception {
        TlsPeerManager peerManager = EasyMock.createMock(TlsPeerManager.class);
        List<TlsPeer> peers = new ArrayList<TlsPeer>();

        TlsPeer peer1 = new TlsPeer();
        peer1.setName("trusteddomain.com");
        InternalUser user1 = new InternalUser();
        user1.setUserName("~~tp~trusteddomain.com");
        peer1.setInternalUser(user1);
        peers.add(peer1);

        TlsPeer peer2 = new TlsPeer();
        peer2.setName("10.10.1.2");
        InternalUser user2 = new InternalUser();
        user2.setUserName("~~tp~10.10.1.2");
        peer2.setInternalUser(user2);
        peers.add(peer2);

        peerManager.getTlsPeers();
        EasyMock.expectLastCall().andReturn(peers);
        EasyMock.replay(peerManager);

        PeerIdentitiesConfiguration peerIdentitiesConfiguration = new PeerIdentitiesConfiguration();
        peerIdentitiesConfiguration.setTlsPeerManager(peerManager);

        Document document = peerIdentitiesConfiguration.getDocument();
        String domDoc = TestUtil.asString(document);

        InputStream referenceXml = PeerIdentitiesConfiguration.class.getResourceAsStream("peeridentities.test.xml");
        assertEquals(IOUtils.toString(referenceXml), domDoc);
    }
}
