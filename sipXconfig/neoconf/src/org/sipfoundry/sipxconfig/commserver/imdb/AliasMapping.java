/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import static org.sipfoundry.commons.mongo.MongoConstants.ALIAS_ID;
import static org.sipfoundry.commons.mongo.MongoConstants.CONTACT;
import static org.sipfoundry.commons.mongo.MongoConstants.RELATION;;

/**
 * AliasMapping defines the relationships between an identity and a contact. For example (in
 * context of hunt group): (identity -> contact) 300 -> sip:sales@kuku sales -> sip:user-one@kuku
 * sales -> sip:user-two@kuku 'relation' gives the nature of the source of the mapping. See
 * sipXregistry/meta/alias.xsd.in for valid values.
 */
public class AliasMapping extends DataSetRecord {

    public AliasMapping() {
        // empty default
    }

    /**
     * @param identity
     * @param contact
     */
    public AliasMapping(String identity, String contact, String relation) {
        put(ALIAS_ID, identity);
        put(CONTACT, contact);
        put(RELATION, relation);
    }

    // convenience methods
    public String getIdentity() {
        return get(ALIAS_ID).toString();
    }

    public String getContact() {
        if (get(CONTACT) != null) {
            return get(CONTACT).toString();
        }
        return null;
    }

    public void setIdentity(String ident) {
        put(ALIAS_ID, ident);
    }

    public void setContact(String cnt) {
        put(CONTACT, cnt);
    }

    public static String createUri(String user, String domain) {
        return user + "@" + domain;
    }

}
