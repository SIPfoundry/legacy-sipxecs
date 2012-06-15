/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.site.admin.ldap;

import java.util.List;

import org.sipfoundry.sipxconfig.bulk.ldap.LdapConnectionParams;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;

public class LdapSelectionModel extends ObjectSelectionModel {
    private LdapManager m_ldapManager;

    public LdapSelectionModel(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
        List<LdapConnectionParams> connections = m_ldapManager.getAllConnectionParams();
        setCollection(connections);
        setLabelExpression("url");
        setValueExpression("id");
    }
}
