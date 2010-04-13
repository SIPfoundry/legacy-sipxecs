/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.alias;

import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * AliasManager: manages all SIP aliases
 */
public interface AliasManager extends AliasOwner {
    public static final String CONTEXT_BEAN_NAME = "aliasManager";

    /**
     * Return true if the bean is allowed to use the specified alias.
     * If there are no existing database objects with that alias, then the
     * bean is allowed to use the alias.
     * If there are existing database objects with that alias, then the bean is
     * only allowed to use the alias if one of those objects is the bean itself.
     * (Ideally there should be at most one database object with a given alias, but
     * it is quite possible for duplication to occur across tables and the
     * system needs to continue to operate smoothly in this situation.)
     * If alias is null, then return true, since null aliases don't cause SIP collisions.
     */
    boolean canObjectUseAlias(BeanWithId bean, String alias);

}
