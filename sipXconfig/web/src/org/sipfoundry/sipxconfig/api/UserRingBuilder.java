/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.util.Arrays;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.CoreContext;

public class UserRingBuilder extends AbstractRingBuilder {
    private static final String USER_NAME_PROP = "userName";
    private static final String[] IGNORE_LIST = {
        USER_NAME_PROP
    };
    private CoreContext m_coreContext;

    public UserRingBuilder(CoreContext coreContext) {
        super();
        m_coreContext = coreContext;
        getCustomFields().addAll(Arrays.asList(IGNORE_LIST));
    }

    public void toApiObject(Object apiObject, Object myObject, Set properties) {
        super.toApiObject(apiObject, myObject, properties);
        org.sipfoundry.sipxconfig.admin.callgroup.UserRing my =
            (org.sipfoundry.sipxconfig.admin.callgroup.UserRing) myObject;
        UserRing api = (UserRing) apiObject;
        if (properties.contains(USER_NAME_PROP)) {
            api.setUserName(my.getUser().getUserName());
        }
    }

    public void toMyObject(Object myObject, Object apiObject, Set properties) {
        super.toMyObject(myObject, apiObject, properties);
        org.sipfoundry.sipxconfig.admin.callgroup.UserRing my =
            (org.sipfoundry.sipxconfig.admin.callgroup.UserRing) myObject;
        UserRing api = (UserRing) apiObject;
        if (properties.contains(USER_NAME_PROP)) {
            String userName = api.getUserName();
            // TODO: Give more thought to error handling and whether these exceptions will get handled properly.
            // Presumably we should send a SOAP fault back to the SOAP client.
            if (userName == null || userName.length() == 0) {
                throw new RuntimeException("Cannot create a user ring given a null or empty userName");
            }
            org.sipfoundry.sipxconfig.common.User user = m_coreContext.loadUserByUserName(userName);
            if (user == null) {
                throw new RuntimeException("Cannot find user with userName: " + userName);
            }
            my.setUser(user);
        }
    }
}
