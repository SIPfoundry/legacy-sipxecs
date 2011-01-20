/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

/**
 * AliasMapping defines the relationships between an identity and a contact. For example (in
 * context of hunt group): (identity -> contact) 300 -> sip:sales@kuku sales -> sip:user-one@kuku
 * sales -> sip:user-two@kuku 'relation' gives the nature of the source of the mapping. See
 * sipXregistry/meta/alias.xsd.in for valid values.
 */
public class AliasMapping extends DataSetRecord {

    public static final String IDENTITY = "id";
    public static final String CONTACT = "cnt";

    private String m_identity;
    private String m_contact;

    public AliasMapping() {
        // empty default
    }

    /**
     * @param identity
     * @param contact
     */
    public AliasMapping(String identity, String contact) {
        put(IDENTITY, identity);
        put(CONTACT, contact);
    }

    public AliasMapping(String identity) {
        put(IDENTITY, identity);
    }

    // convenience methods
    public String getIdentity() {
        return get(IDENTITY).toString();
    }

    public String getContact() {
        if (get(CONTACT) != null) {
            return get(CONTACT).toString();
        }
        return null;
    }

    public void setIdentity(String ident) {
        put(IDENTITY, ident);
    }

    public void setContact(String cnt) {
        put(CONTACT, cnt);
    }

    public static String createUri(String user, String domain) {
        return user + "@" + domain;
    }

}
