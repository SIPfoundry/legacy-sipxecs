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

import org.sipfoundry.sipxconfig.admin.callgroup.AbstractCallSequence;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipxCollectionUtils;

public class CallGroupBuilder extends SimpleBeanBuilder {
    private static final String RINGS_PROP = AbstractCallSequence.RINGS_PROP;
    private static final String[] IGNORE_LIST = {
        RINGS_PROP
    };
    private CoreContext m_coreContext;

    public CallGroupBuilder() {
        super();
        getCustomFields().addAll(Arrays.asList(IGNORE_LIST));
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void toApiObject(Object apiObject, Object myObject, Set properties) {
        super.toApiObject(apiObject, myObject, properties);
        org.sipfoundry.sipxconfig.admin.callgroup.CallGroup my =
            (org.sipfoundry.sipxconfig.admin.callgroup.CallGroup) myObject;
        CallGroup api = (CallGroup) apiObject;
        if (properties.contains(RINGS_PROP) && !SipxCollectionUtils.safeIsEmpty(my.getRings())) {
            UserRing[] rings =
                (UserRing[]) ApiBeanUtil.toApiArray(new UserRingBuilder(m_coreContext),
                                                    my.getRings(),
                                                    UserRing.class);
            api.setRings(rings);
        }
    }

    public void toMyObject(Object myObject, Object apiObject, Set properties) {
        super.toMyObject(myObject, apiObject, properties);
        org.sipfoundry.sipxconfig.admin.callgroup.CallGroup my =
            (org.sipfoundry.sipxconfig.admin.callgroup.CallGroup) myObject;
        CallGroup api = (CallGroup) apiObject;
        UserRing[] apiRings = api.getRings();
        if (properties.contains(RINGS_PROP)) {
            my.getRings().clear();
            if (apiRings != null) {
                org.sipfoundry.sipxconfig.admin.callgroup.UserRing[] myRings =
                    (org.sipfoundry.sipxconfig.admin.callgroup.UserRing[])
                    ApiBeanUtil.toMyArray(new UserRingBuilder(m_coreContext), apiRings,
                            org.sipfoundry.sipxconfig.admin.callgroup.UserRing.class);
                my.insertRings(Arrays.asList(myRings));
            }
        }
    }
}
