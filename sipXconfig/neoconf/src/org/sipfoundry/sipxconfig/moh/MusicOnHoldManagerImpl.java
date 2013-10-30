/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.moh;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.common.BeanId;
import org.springframework.beans.factory.annotation.Required;

public class MusicOnHoldManagerImpl extends AbstractMusicOnHoldManagerImpl {

    private String m_mohUser;

    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        if (alias.startsWith(m_mohUser)) {
            // id = 1 because there's only 1 MOH instance
            return BeanId.createBeanIdCollection(Collections.singleton(1), this.getClass());
        }
        return Collections.emptyList();
    }

    @Required
    public void setMohUser(String mohUser) {
        m_mohUser = mohUser;
    }

    public boolean isAliasInUse(String alias) {

        if (alias.startsWith(m_mohUser)) {
            return true;
        }
        return false;
    }

}
