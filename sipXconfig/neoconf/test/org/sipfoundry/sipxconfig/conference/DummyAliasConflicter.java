/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
