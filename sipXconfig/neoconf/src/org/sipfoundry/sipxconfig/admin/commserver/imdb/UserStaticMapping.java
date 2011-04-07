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

import static org.sipfoundry.commons.mongo.MongoConstants.*;

public class UserStaticMapping extends DataSetRecord {

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
