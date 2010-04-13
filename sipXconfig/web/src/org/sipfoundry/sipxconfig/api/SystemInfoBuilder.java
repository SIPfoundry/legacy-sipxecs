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
import java.util.TreeSet;

public class SystemInfoBuilder extends SimpleBeanBuilder {
    private static final String ALIASES_PROP = "aliases";
    private static final String REALM_PROP = "realm";

    private static final String[] IGNORE_LIST = {
        ALIASES_PROP, REALM_PROP
    };

    public SystemInfoBuilder() {
        getCustomFields().addAll(Arrays.asList(IGNORE_LIST));
    }

    public void toApiObject(Object apiObject, Object myObject, Set properties) {
        super.toApiObject(apiObject, myObject, properties);
        org.sipfoundry.sipxconfig.domain.Domain my = (org.sipfoundry.sipxconfig.domain.Domain) myObject;
        Domain api = (Domain) apiObject;
        if (properties.contains(ALIASES_PROP) && my.hasAliases()) {
            api.setAliases(my.getAliases().toArray(new String[0]));
        }
    }

    public void toMyObject(Object myObject, Object apiObject, Set properties) {
        super.toMyObject(myObject, apiObject, properties);
        org.sipfoundry.sipxconfig.domain.Domain my = (org.sipfoundry.sipxconfig.domain.Domain) myObject;
        Domain api = (Domain) apiObject;
        String[] aliases = api.getAliases();
        if (properties.contains(ALIASES_PROP) && aliases != null) {
            my.setAliases(new TreeSet<String>(Arrays.asList(api.getAliases())));
        }
    }
}
