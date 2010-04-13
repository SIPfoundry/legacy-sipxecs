/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.callgroup;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.alias.AliasOwner;

public interface CallGroupContext extends AliasOwner, AliasProvider {
    public static final String CONTEXT_BEAN_NAME = "callGroupContext";

    void activateCallGroups();

    CallGroup loadCallGroup(Integer id);

    List<CallGroup> getCallGroups();

    void storeCallGroup(CallGroup callGroup);

    void removeCallGroups(Collection ids);

    void duplicateCallGroups(Collection ids);

    void removeUser(Integer userId);

    void addUsersToCallGroup(Integer callGroupId, Collection ids);

    void clear();

    void generateSipPasswords();
}
