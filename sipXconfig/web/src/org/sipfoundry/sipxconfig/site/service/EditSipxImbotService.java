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
package org.sipfoundry.sipxconfig.site.service;

import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;

public abstract class EditSipxImbotService extends EditSipxService {
    @SuppressWarnings("hiding")
    public static final String PAGE = "service/EditSipxImbotService";

    @InjectObject("spring:ldapManager")
    public abstract LdapManager getLdapManager();

    @Override
    public void apply() {
        super.apply();
        //we need to acknowledge openfire that mybuddy credentials has changed
//        getLdapManager().replicateOpenfireConfig();
    }
}
