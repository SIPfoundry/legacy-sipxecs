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

    private List m_rings = new ArrayList();

    public AbstractCallSequence() {
        // empty default constructor
    }

    /**
     * Deep clone. Hiberate friendly.
     */
    protected Object clone() throws CloneNotSupportedException {
        AbstractCallSequence clone = (AbstractCallSequence) super.clone();
        clone.m_rings = new ArrayList(m_rings.size());
        DataCollectionUtil.duplicate(m_rings, clone.m_rings);
        return clone;
    }

    protected void insertRing(AbstractRing ring) {
        m_rings.add(ring);
        DataCollectionUtil.updatePositions(m_rings);
    }

    protected void insertRings(Collection rings) {
        for (Iterator iter = rings.iterator(); iter.hasNext();) {
            AbstractRing ring = (AbstractRing) iter.next();
            m_rings.add(ring);
        }
        DataCollectionUtil.updatePositions(m_rings);
    }

    public void removeRings(Collection ids) {
        DataCollectionUtil.removeByPrimaryKey(m_rings, ids.toArray());
    }

    public void moveRings(Collection ids, int step) {
        DataCollectionUtil.moveByPrimaryKey(m_rings, ids.toArray(), step);
    }

    public void removeRing(AbstractRing ringToRemove) {
        Object[] keys = new Object[] {
            ringToRemove.getId()
        };
        DataCollectionUtil.removeByPrimaryKey(m_rings, keys);
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
    public List getRings() {
        return m_rings;
    }

    public void setRings(List rings) {
        m_rings = rings;
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
    protected List generateAliases(String identity, String domain, boolean neverRouteToVoicemail) {

        List rings = getRings();
        
        List aliases = new ArrayList(rings.size());
        ForkQueueValue q = new ForkQueueValue(rings.size());
        for (Iterator i = rings.iterator(); i.hasNext();) {
            AbstractRing r = (AbstractRing) i.next();
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
            String contact = r.calculateContact(domain, q, ignoreVoiceMail);
            AliasMapping alias = new AliasMapping(identity, contact);
            aliases.add(alias);
        }
        return aliases;
    }

    /**
     * Clear all rings for this sequence
     */
    public void clear() {
        m_rings.clear();
    }
}
