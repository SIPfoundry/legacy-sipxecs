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

import org.sipfoundry.sipxconfig.common.SipxCollectionUtils;

public class ConferenceBridgeBuilder extends SimpleBeanBuilder {
    private static final String CONFERENCES_PROP =
        org.sipfoundry.sipxconfig.conference.Bridge.CONFERENCES_PROP;
    // The serviceUri is read-only.  Intercept this property to ensure we only read it and
    // never try to write it.
    private static final String SERVICE_URI_PROP =
        org.sipfoundry.sipxconfig.conference.Bridge.SERVICE_URI_PROP;
    private static final String[] IGNORE_LIST = { 
        CONFERENCES_PROP, SERVICE_URI_PROP
    };

    public ConferenceBridgeBuilder() {
        super();
        getCustomFields().addAll(Arrays.asList(IGNORE_LIST));
    }

    public void toApiObject(Object apiObject, Object myObject, Set properties) {
        super.toApiObject(apiObject, myObject, properties);
        org.sipfoundry.sipxconfig.conference.Bridge my =
            (org.sipfoundry.sipxconfig.conference.Bridge) myObject;
        ConferenceBridge api = (ConferenceBridge) apiObject;
        if (properties.contains(CONFERENCES_PROP) && !SipxCollectionUtils.safeIsEmpty(my.getConferences())) {
            Conference[] conferences =
                (Conference[]) ApiBeanUtil.toApiArray(new SimpleBeanBuilder(), my.getConferences(), Conference.class);
            api.setConferences(conferences);
        }
        if (properties.contains(SERVICE_URI_PROP)) {
            api.setServiceUri(my.getServiceUri());
        }
    }

    public void toMyObject(Object myObject, Object apiObject, Set properties) {
        super.toMyObject(myObject, apiObject, properties);
        org.sipfoundry.sipxconfig.conference.Bridge my =
            (org.sipfoundry.sipxconfig.conference.Bridge) myObject;
        ConferenceBridge api = (ConferenceBridge) apiObject;
        if (properties.contains(CONFERENCES_PROP)) {
            Conference[] apiConferences = api.getConferences();
            my.getConferences().clear();
            if (apiConferences != null) {
                org.sipfoundry.sipxconfig.conference.Conference[] myConferences = 
                    (org.sipfoundry.sipxconfig.conference.Conference[])
                    ApiBeanUtil.toMyArray(new SimpleBeanBuilder(), apiConferences,
                            org.sipfoundry.sipxconfig.conference.Conference.class);
                my.addConferences(Arrays.asList(myConferences));
            }
        }
    }

}
