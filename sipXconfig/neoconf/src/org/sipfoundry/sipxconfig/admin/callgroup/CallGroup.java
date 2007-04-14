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

import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.common.User;

public class CallGroup extends AbstractCallSequence implements NamedObject {
    private boolean m_enabled;
    private String m_name;
    private String m_extension;
    private String m_description;

    public CallGroup() {
        // bean usage only
    }

    /**
     * We need a deep clone. Each call can only belong to single collection.
     */
    protected Object clone() throws CloneNotSupportedException {
        CallGroup clone = (CallGroup) super.clone();
        clone.clearRings();
        for (Iterator i = getRings().iterator(); i.hasNext();) {
            UserRing ring = (UserRing) i.next();
            UserRing ringClone = (UserRing) ring.duplicate();
            ringClone.setCallGroup(clone);
            clone.insertRing(ringClone);
        }
        return clone;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    /**
     * Inserts a new ring for a specific user
     * 
     * @param user
     * @return newly created user ring
     */
    public UserRing insertRingForUser(User user) {
        UserRing ring = new UserRing();
        ring.setCallGroup(this);
        ring.setUser(user);
        insertRing(ring);
        return ring;
    }

    public void insertRings(Collection rings) {
        super.insertRings(rings);
        for (Iterator iter = rings.iterator(); iter.hasNext();) {
            UserRing ring = (UserRing) iter.next();
            ring.setCallGroup(this);
        }
    }
    
    /**
     * Create list of aliases that descibe forwarding for this group. All aliases have the form
     * group_name@domain => user_name@domain with the specific q value In addtion alias extension =>
     * group name is added to allow calling to extension
     * 
     * @return list of AliasMapping objects (identity => contact)
     */
    public List generateAliases(String domainName) {
        if (!isEnabled()) {
            return Collections.EMPTY_LIST;
        }
        String myIdentity = AliasMapping.createUri(m_name, domainName);

        List aliases = generateAliases(myIdentity, domainName, false);
        if (StringUtils.isNotBlank(m_extension) && !m_extension.equals(m_name)) {
            AliasMapping extensionAlias = new AliasMapping(AliasMapping.createUri(m_extension,
                    domainName), myIdentity);
            aliases.add(extensionAlias);
        }
        return aliases;
    }

}
