/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import static org.sipfoundry.commons.mongo.MongoConstants.CALLID;
import static org.sipfoundry.commons.mongo.MongoConstants.CONTACT;
import static org.sipfoundry.commons.mongo.MongoConstants.EVENT;
import static org.sipfoundry.commons.mongo.MongoConstants.FROM_URI;
import static org.sipfoundry.commons.mongo.MongoConstants.IDENTITY;
import static org.sipfoundry.commons.mongo.MongoConstants.TO_URI;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.SipUri;

public class UserStaticMapping extends DataSetRecord {
    private static final String AT_SIGN = "@";

    public UserStaticMapping(String domain, String username, String mwi) {
        String identity = username + AT_SIGN + domain;
        put(IDENTITY, identity);
        put(EVENT, "message-summary");
        if (StringUtils.contains(mwi, AT_SIGN)) {
            String[] mwiParts = StringUtils.split(mwi, AT_SIGN);
            put(CONTACT, SipUri.format(mwiParts[0], mwiParts[1], false));
        } else {
            put(CONTACT, SipUri.format(mwi, domain, false));
        }
        put(FROM_URI, SipUri.format("IVR", domain, false));
        put(TO_URI, SipUri.format(username, domain, false));
        put(CALLID, "static-mwi-" + identity);
    }

}
