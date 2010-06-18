/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.forwarding;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.apache.commons.lang.builder.ToStringBuilder;

/**
 * AliasMapping defines the relationships between an identity and
 * a contact.  For example (in context of hunt group):
 * (identity ->  contact)
 * 300      ->  sip:sales@kuku
 * sales    ->  sip:user-one@kuku
 * sales    ->  sip:user-two@kuku
 * 'relation' gives the nature of the source of the mapping.
 * See sipXregistry/meta/alias.xsd.in for valid values.
 */
public class AliasMapping {
    private String m_identity;
    private String m_contact;
    private String m_relation;

    public AliasMapping() {
        // empty default
    }

    /**
     * @param identity
     * @param contact
     * @param relation
     */
    public AliasMapping(String identity, String contact, String relation) {
        this.m_identity = identity;
        this.m_contact = contact;
        this.m_relation = relation;
    }

    public synchronized String getContact() {
        return m_contact;
    }

    public synchronized void setContact(String contact) {
        this.m_contact = contact;
    }

    public synchronized String getIdentity() {
        return m_identity;
    }

    public synchronized void setIdentity(String identity) {
        this.m_identity = identity;
    }

    public synchronized String getRelation() {
        return m_relation;
    }

    public synchronized void setRelation(String relation) {
        this.m_relation = relation;
    }

    public String toString() {
        ToStringBuilder builder = new ToStringBuilder(this);
        builder.append("identity", m_identity);
        builder.append("contact", m_contact);
        builder.append("relation", m_relation);
        return builder.toString();
    }

    public boolean equals(Object obj) {
        if (!(obj instanceof AliasMapping)) {
            return false;
        }
        if (this == obj) {
            return true;
        }
        AliasMapping rhs = (AliasMapping) obj;
        EqualsBuilder builder = new EqualsBuilder();
        builder.append(m_identity, rhs.m_identity);
        builder.append(m_contact, rhs.m_contact);
        builder.append(m_relation, rhs.m_relation);
        return builder.isEquals();
    }

    public int hashCode() {
        return new HashCodeBuilder().append(m_identity).append(m_contact).append(m_relation).toHashCode();
    }

    public static String createUri(String user, String domain) {
        return user + "@" + domain;
    }
}
