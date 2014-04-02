/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.tls;

import static java.util.Collections.singletonList;
import static org.sipfoundry.sipxconfig.common.DaoUtils.requireOneOrZero;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.RandomStringUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.springframework.beans.factory.annotation.Required;

public class TlsPeerManagerImpl extends SipxHibernateDaoSupport implements TlsPeerManager {

    private static final String TLS_PEER_NAME = "name";
    private static final String INTERNAL_NAME = "~~tp~%s";
    private CoreContext m_coreContext;

    @Override
    public void deleteTlsPeer(TlsPeer tlsPeer) {
        deleteTlsPeers(singletonList(tlsPeer.getId()));
    }

    @Override
    public void deleteTlsPeers(Collection<Integer> allSelected) {
        List<TlsPeer> peers = getTlsPeers(allSelected);
        for (TlsPeer peer : peers) {
            getDaoEventPublisher().publishDelete(peer);
        }
        getHibernateTemplate().deleteAll(peers);
    }

    @Override
    public TlsPeer getTlsPeer(Integer tlsPeerId) {
        return (TlsPeer) getHibernateTemplate().load(TlsPeer.class, tlsPeerId);
    }

    @Override
    public List<TlsPeer> getTlsPeers() {
        return getHibernateTemplate().loadAll(TlsPeer.class);
    }

    @Override
    public List<TlsPeer> getTlsPeers(Collection<Integer> ids) {
        List<TlsPeer> peers = new ArrayList<TlsPeer>(ids.size());
        for (Integer id : ids) {
            peers.add(getTlsPeer(id));
        }
        return peers;
    }

    @Override
    public void saveTlsPeer(TlsPeer tlsPeer) {
        // check if tls peer name is empty
        if (StringUtils.isBlank(tlsPeer.getName())) {
            throw new UserException("&blank.peerName.error");
        }
        // Check for duplicate names before saving the peer
        if (tlsPeer.isNew() || (!tlsPeer.isNew() && isNameChanged(tlsPeer))) {
            checkForDuplicateName(tlsPeer);
        }
        String userName = StringUtils.deleteWhitespace(String.format(INTERNAL_NAME, tlsPeer.getName()));
        tlsPeer.getInternalUser().setUserName(userName);
        if (!tlsPeer.isNew()) {
            getHibernateTemplate().merge(tlsPeer);
        } else {
            getHibernateTemplate().save(tlsPeer);
        }
    }

    @Override
    public TlsPeer getTlsPeerByName(String name) {
        String query = "tlsPeerByName";
        Collection<TlsPeer> peers = getHibernateTemplate().findByNamedQueryAndNamedParam(query, TLS_PEER_NAME, name);
        return requireOneOrZero(peers, query);
    }

    @Override
    public List<Replicable> getReplicables() {
        List<Replicable> replicables = new ArrayList<Replicable>();
        for (TlsPeer tp : getTlsPeers()) {
            replicables.add(tp);
        }
        return replicables;
    }

    public TlsPeer newTlsPeer() {
        TlsPeer peer = new TlsPeer();
        InternalUser internaluser = m_coreContext.newInternalUser();
        internaluser.setSipPassword(RandomStringUtils.randomAlphanumeric(10));
        internaluser.setSettingTypedValue(PermissionName.VOICEMAIL.getPath(), false);
        internaluser.setSettingTypedValue(PermissionName.FREESWITH_VOICEMAIL.getPath(), false);
        peer.setInternalUser(internaluser);
        return peer;
    }

    private void checkForDuplicateName(TlsPeer peer) {
        String peerName = peer.getName();
        TlsPeer existingPeer = getTlsPeerByName(peerName);
        if (existingPeer != null) {
            throw new UserException("&duplicate.peerName.error", peerName);
        }
    }


    private boolean isNameChanged(TlsPeer peer) {
        return !getTlsPeer(peer.getId()).getName().equals(peer.getName());
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

}
