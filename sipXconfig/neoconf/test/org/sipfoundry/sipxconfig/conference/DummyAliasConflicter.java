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
package org.sipfoundry.sipxconfig.conference;


import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.BeanId;

public class DummyAliasConflicter implements AliasOwner {    
    public static final String MY_ALIAS = "858585";
    
    @Override
    public boolean isAliasInUse(String alias) {
        return MY_ALIAS.equals(alias);
    }

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        if (MY_ALIAS.equals(alias)) {
            return BeanId.createBeanIdCollection(Collections.singleton(1), DummyAliasConflicter.class);
        }
        return Collections.emptyList();
    }
}
