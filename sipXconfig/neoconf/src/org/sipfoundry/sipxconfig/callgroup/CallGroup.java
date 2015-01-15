/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.callgroup;

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.RandomStringUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.commons.security.Md5Encoder;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.dialplan.ForkQueueValue;
import org.sipfoundry.sipxconfig.dialplan.MappingRule;
import org.sipfoundry.sipxconfig.systemaudit.SystemAuditable;

public class CallGroup extends AbstractCallSequence implements Replicable, SystemAuditable {
    private static final int SIP_PASSWORD_LEN = 10;
    private static final String ALIAS_RELATION = "callgroup";
    private boolean m_enabled;
    private String m_name;
    private String m_extension;
    private String m_did;
    private String m_description;
    private String m_fallbackDestination;
    private boolean m_voicemailFallback = true;
    private boolean m_userForward = true;
    private String m_sipPassword;
    private boolean m_useFwdTimers;
    private Integer m_fallbackExpire = AbstractRing.DEFAULT_EXPIRATION;

    public CallGroup() {
        generateSipPassword();
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

    public String calculateUri(String domainName) {
        return SipUri.format(getName(), domainName, false);
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

    public String getFallbackDestination() {
        return m_fallbackDestination;
    }

    public void setFallbackDestination(String fallbackDestination) {
        m_fallbackDestination = fallbackDestination;
    }

    public boolean getVoicemailFallback() {
        return m_voicemailFallback;
    }

    public void setVoicemailFallback(boolean voicemailFallback) {
        m_voicemailFallback = voicemailFallback;
    }

    public boolean getUserForward() {
        return m_userForward;
    }

    public void setUserForward(boolean userForward) {
        m_userForward = userForward;
    }

    public boolean getUseFwdTimers() {
        return m_useFwdTimers;
    }

    public void setUseFwdTimers(boolean useFwdTimers) {
        m_useFwdTimers = useFwdTimers;
    }

    public String getSipPassword() {
        return m_sipPassword;
    }

    public void setSipPassword(String sipPassword) {
        m_sipPassword = sipPassword;
    }

    public Integer getFallbackExpire() {
        return m_fallbackExpire;
    }

    public void setFallbackExpire(Integer expire) {
        m_fallbackExpire = expire;
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
     * group_name@domain => user_name@domain with the specific q value. In addtion alias extension
     * => group name is added to allow calling to extension
     *
     * @return list of AliasMapping objects (identity => contact)
     */
    public List<AliasMapping> getAliasMappings(String domainName) {
        if (!isEnabled()) {
            return Collections.emptyList();
        }
        String myIdentity = AliasMapping.createUri(m_name, domainName);

        ForkQueueValue forkQueueValue = new ForkQueueValue(getRings().size() + 1);
        List<AliasMapping> mappings = generateAliases(m_name, m_extension, domainName, true,
                m_userForward, forkQueueValue);

        if (m_voicemailFallback) {
            AbstractRing lastRing = getLastRing();
            if (lastRing != null) {
                if (AbstractRing.Type.IMMEDIATE == lastRing.getType()) {
                    // the last ring is a parallel ring
                    // decrement q so that VM contact has lower q than the last contact
                    forkQueueValue.getSerial();
                }
                String vmailContact = lastRing.calculateContact(domainName, null, forkQueueValue, false, m_userForward,
                        MappingRule.Voicemail.VM_PREFIX);
                mappings.add(new AliasMapping(m_name, vmailContact, ALIAS_RELATION));
            }
        } else if (StringUtils.isNotBlank(m_fallbackDestination)) {
            String falback = SipUri.fix(m_fallbackDestination, domainName);
            String fallbackContact = String.format("<%s?expires=%s>;%s", falback, m_fallbackExpire,
                    forkQueueValue.getSerial());
            mappings.add(new AliasMapping(m_name, fallbackContact, ALIAS_RELATION));
        }

        if (StringUtils.isNotBlank(m_extension) && !m_extension.equals(m_name)) {
            AliasMapping extensionAlias = new AliasMapping(m_extension, myIdentity, ALIAS_RELATION);
            mappings.add(extensionAlias);
        }
        if (StringUtils.isNotBlank(m_did) && !m_did.equals(m_name)) {
            AliasMapping didAlias = new AliasMapping(m_did, myIdentity, ALIAS_RELATION);
            mappings.add(didAlias);
        }

        return mappings;
    }

    public String getSipPasswordHash(String realm) {
        String password = StringUtils.defaultString(m_sipPassword);
        return Md5Encoder.digestEncryptPassword(m_name, realm, password);
    }

    /**
     * Generates a new SIP password for the call group. If the password already exist this method
     * does not change it.
     *
     * @return true is the passwors has been generated/changed
     */
    public boolean generateSipPassword() {
        if (StringUtils.isNotEmpty(m_sipPassword)) {
            return false;
        }
        m_sipPassword = RandomStringUtils.randomAlphanumeric(SIP_PASSWORD_LEN);
        return true;
    }

    public String getDid() {
        return m_did;
    }

    public void setDid(String did) {
        m_did = did;
    }

    @Override
    public Set<DataSet> getDataSets() {
        Set<DataSet> ds = new HashSet<DataSet>();
        ds.add(DataSet.ALIAS);
        ds.add(DataSet.CREDENTIAL);
        return ds;
    }

    @Override
    public String getIdentity(String domain) {
        return SipUri.stripSipPrefix(SipUri.format(null, getName(), domain));
    }

    @Override
    public boolean isValidUser() {
        return true;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        return Collections.emptyMap();
    }

    @Override
    public String getEntityName() {
        return getClass().getSimpleName();
    }

    /**
     * CallGroup entity must be replicated only when m_enabled is set to true
     */
    @Override
    public boolean isReplicationEnabled() {
        return isEnabled();
    }

    @Override
    public List<AbstractRing> getRings() {
        List<AbstractRing> rings = super.getRings();
        if (m_userForward && m_useFwdTimers) {
            for (AbstractRing ring : rings) {
                if (ring instanceof UserRing) {
                    ring.setExpiration(((UserRing) ring).getUser().getEffectiveExpire());
                }
            }
        }
        return rings;
    }

    @Override
    public String getEntityIdentifier() {
        return getName();
    }

    @Override
    public String getConfigChangeType() {
        return CallGroup.class.getSimpleName();
    }
}
