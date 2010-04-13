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

public class PermissionBuilder extends SimpleBeanBuilder {
    private static final String BUILTIN_PROP = "builtIn";
    private static final String NAME_PROP = "name";
    private static final String TYPE_PROP = "type";

    private static final String[] CUSTOM_FIELDS = {
        ID_PROP, BUILTIN_PROP, TYPE_PROP, NAME_PROP
    };

    public PermissionBuilder() {
        getCustomFields().addAll(Arrays.asList(CUSTOM_FIELDS));
    }

    public void toApiObject(Object apiObject, Object myObject, Set properties) {
        super.toApiObject(apiObject, myObject, properties);
        Permission apiPerm = (Permission) apiObject;
        org.sipfoundry.sipxconfig.permission.Permission myPerm =
            (org.sipfoundry.sipxconfig.permission.Permission) myObject;
        if (properties.contains(BUILTIN_PROP)) {
            apiPerm.setBuiltIn(myPerm.isBuiltIn());
        }
        if (properties.contains(NAME_PROP)) {
            apiPerm.setName(myPerm.getName());
        }
        if (properties.contains(TYPE_PROP)) {
            apiPerm.setType(myPerm.getType().toString());
        }
    }
}
