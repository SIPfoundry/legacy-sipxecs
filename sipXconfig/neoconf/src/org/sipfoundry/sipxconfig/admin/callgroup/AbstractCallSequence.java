/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.callgroup;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing.Type;
import org.sipfoundry.sipxconfig.admin.dialplan.ForkQueueValue;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;

/**
 * CallSequence
 */
public class AbstractCallSequence extends BeanWithId {
    public static final String RINGS_PROP = "rings";

    private List<AbstractRing> m_rings = new ArrayList<AbstractRing>();

    public AbstractCallSequence() {
        // empty default constructor
    }

    /**
     * Deep clone. Hiberate friendly.
     */
    @Override
    protected Object clone() throws CloneNotSupportedException {
        AbstractCallSequence clone = (AbstractCallSequence) super.clone();
        clone.m_rings = new ArrayList(m_rings.size());
        DataCollectionUtil.duplicate(m_rings, clone.m_rings);
        return clone;
    }

    protected void insertRing(AbstractRing ring) {
        m_rings.add(ring);
    }

    protected void insertRings(Collection< ? extends AbstractRing> rings) {
        m_rings.addAll(rings);
    }

    public void replaceRings(Collection< ? extends AbstractRing> rings) {
        clearRings();
        insertRings(rings);
    }

    public void removeRings(Collection ids) {
        DataCollectionUtil.removeByPrimaryKey(m_rings, ids.toArray());
    }

    public void moveRings(Collection ids, int step) {
        DataCollectionUtil.moveByPrimaryKey(m_rings, ids.toArray(), step);
    }

    public void removeRing(AbstractRing ringToRemove) {
        DataCollectionUtil.removeByPrimaryKey(m_rings, ringToRemove.getId());
    }

    protected void clearRings() {
        m_rings.clear();
    }

    public boolean moveRingUp(AbstractRing ring) {
        Object[] keys = new Object[] {
            ring.getId()
        };
        DataCollectionUtil.moveByPrimaryKey(m_rings, keys, -1);
        return true;
    }

    public boolean moveRingDown(AbstractRing ring) {
        Object[] keys = new Object[] {
            ring.getId()
        };
        DataCollectionUtil.moveByPrimaryKey(m_rings, keys, 1);
        return true;
    }

    /**
     * Return the list of rings. Don't alter the list directly, always call a method on this
     * interface (or a derived class) to change the rings list.
     */
    public List<AbstractRing> getRings() {
        return m_rings;
    }

    public void setRings(List<AbstractRing> rings) {
        m_rings = rings;
    }

    /**
     * @return last ring in the sequence or null if sequence is empty
     */
    public AbstractRing getLastRing() {
        if (m_rings.isEmpty()) {
            return null;
        }
        return m_rings.get(m_rings.size() - 1);
    }

    /**
     * Generate aliases from the calling list. All aliases have the following form: identity ->
     * ring_contact
     *
     * @param identity
     * @param domain used to calculate proper URI for ring contact
     * @param neverRouteToVoicemail set to true if call should never be routed to voicemail, set
     *        to false to allow for routing the last call to voicemail
     * @param q allows calling method to control size of fork queue value. Should be initizlized
     *        with a number at least as large as the size of the rings list
     * @return list of AliasMapping objects
     */
    protected List<AliasMapping> generateAliases(String identity, String domain, boolean neverRouteToVoicemail,
            boolean userForward, ForkQueueValue q) {

        List<AbstractRing> rings = getRings();

        List<AliasMapping> aliases = new ArrayList<AliasMapping>(rings.size());
        for (Iterator<AbstractRing> i = rings.iterator(); i.hasNext();) {
            AbstractRing r = i.next();
            if (StringUtils.isEmpty(r.getUserPart().toString()) || !r.isEnabled()) {
                continue;
            }
            // ignore voice mail if neverRouteToVoicemail is set
            boolean ignoreVoiceMail = neverRouteToVoicemail;
            // if not already ignored - ignore it for all but last stage
            if (!ignoreVoiceMail) {
                ignoreVoiceMail = i.hasNext();
            }
            // XCF-1026 if not already ignored - ignore it for IMMEDIATE (parallel) forks
            if (!ignoreVoiceMail) {
                ignoreVoiceMail = Type.IMMEDIATE.equals(r.getType());
            }
            String contact = r.calculateContact(domain, q, ignoreVoiceMail, userForward, null);
            AliasMapping alias = new AliasMapping(identity, contact, "userforward");
            aliases.add(alias);
        }
        return aliases;
    }

    /**
     * Generate aliases from the calling list. All aliases have the following form: identity ->
     * ring_contact
     *
     * @param identity
     * @param domain used to calculate proper URI for ring contact
     * @param neverRouteToVoicemail set to true if call should never be routed to voicemail, set
     *        to false to allow for routing the last call to voicemail
     * @return list of AliasMapping objects
     */
    protected List<AliasMapping> generateAliases(String identity, String domain, boolean neverRouteToVoicemail) {
        ForkQueueValue forkQueueValue = new ForkQueueValue(getRings().size());
        return generateAliases(identity, domain, neverRouteToVoicemail, true, forkQueueValue);
    }

    /**
     * Clear all rings for this sequence
     */
    public void clear() {
        m_rings.clear();
    }
}
