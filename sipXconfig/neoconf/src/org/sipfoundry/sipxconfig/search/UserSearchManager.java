/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.search;

import java.util.List;

import org.apache.commons.collections.Transformer;
import org.sipfoundry.sipxconfig.common.User;

public interface UserSearchManager {
    public static final String CONTEXT_BEAN_NAME = "userSearchManager";

    List search(User template, int firstResult, int pageSize, Transformer transformer);
}
