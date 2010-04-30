/*
 *  Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.util.Collection;
import java.util.HashSet;

import org.apache.commons.digester.Digester;
import org.apache.log4j.Logger;
import org.xml.sax.InputSource;

public class PeerIdentities {

    public static final String PEER = "peeridentities/peer";
    private static Logger logger = Logger.getLogger(PeerIdentities.class);

    private static HashSet<Peer> mPeers = new HashSet<Peer>();

    /**
     * Add an ITSP account to the peer database (method is accessed by the digester).
     */
    public void addPeer(Peer peer) {
        if ( logger.isDebugEnabled() ) logger.debug("addPeer: domain " + peer.getTrustedDomain() + ", internal user " + peer.getInternalUser());
        mPeers.add(peer);
    }

    /**
     * Get a collection of peer accounts.
     *
     * @return
     */
    public Collection<Peer> getPeers() {
        return mPeers;
    }

    /**
     * Get the special user identity for a specified peer.
     *
     * @param domain (uri)
     * @return the special user identity for the specified domain, or null if domain is not found.
     */
    public String getUserId(String domain) {
        for (Peer peer : mPeers) {
            if (peer.getTrustedDomain().equalsIgnoreCase(domain)) {
                if ( logger.isDebugEnabled() ) logger.debug("getUserId(" + domain + ") = " + peer.getInternalUser());
                return peer.getInternalUser();
            }
        }
        return null;
    }

    /**
     * Add the digester rules.
     *
     * @param digester
     */
    private static void addRules(Digester digester) {
        digester.addObjectCreate("peeridentities", PeerIdentities.class);
        digester.addObjectCreate(PEER, Peer.class);
        digester.addBeanPropertySetter(PEER + "/trusteddomain", "trustedDomain");
        digester.addBeanPropertySetter(PEER + "/internaluser", "internalUser");
        digester.addSetNext(PEER, "addPeer");
    }

    public PeerIdentities parse(String url) {
        if ( logger.isDebugEnabled() ) logger.debug("parsing peer identities file " + url);
        Digester digester = new Digester();
        digester.setValidating(false);
        addRules(digester);
        try {
            InputSource inputSource = new InputSource(url);
            PeerIdentities peerIdentities = (PeerIdentities)digester.parse(inputSource);
            for (Peer peer : peerIdentities.getPeers()) {
                if ( logger.isDebugEnabled() ) logger.debug("domain: " + peer.getTrustedDomain() + ", internal user " + peer.getInternalUser());
            }
            return peerIdentities;
        } catch (Exception ex) {
            logger.error("caught exception: ", ex);
            throw new RuntimeException(ex);
        }
  }

}
