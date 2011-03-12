/*
 *
 *
 * Copyright (C) 2011 eZuce inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import org.sipfoundry.sipxconfig.common.SipUri;

public class UserStaticMapping extends DataSetRecord {
    public static final String IDENTITY = "ident";
    public static final String EVENT = "evt";
    public static final String CONTACT = "cnt";
    public static final String FROM_URI = "from";
    public static final String TO_URI = "to";
    public static final String CALLID = "cid";

    public UserStaticMapping(String domain, String username, String mwi) {
        String identity = username + "@" + domain;
        put(IDENTITY, identity);
        put(EVENT, "message-summary");
        put(CONTACT, SipUri.format(mwi, domain, false));
        put(FROM_URI, SipUri.format("IVR", domain, false));
        put(TO_URI, SipUri.format(username, domain, false));
        put(CALLID, "static-mwi-" + identity);
    }

}
