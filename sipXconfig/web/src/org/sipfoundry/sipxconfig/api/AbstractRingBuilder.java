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

import org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing.Type;

public class AbstractRingBuilder extends SimpleBeanBuilder {
    // TODO: When enum names are no longer used directly in the UI (see XCF-794),
    // make these strings be the enum names for org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing.Type
    // and use the enum names rather than constants defined here.
    public static final String TYPE_DELAYED = "delayed";
    public static final String TYPE_IMMEDIATE = "immediate";

    private static final String TYPE_PROP =
        org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing.TYPE_PROP;
    private static final String[] IGNORE_LIST = {
        TYPE_PROP
    };

    public AbstractRingBuilder() {
        super();
        getCustomFields().addAll(Arrays.asList(IGNORE_LIST));
    }

    public void toApiObject(Object apiObject, Object myObject, Set properties) {
        super.toApiObject(apiObject, myObject, properties);
        org.sipfoundry.sipxconfig.admin.callgroup.UserRing my =
            (org.sipfoundry.sipxconfig.admin.callgroup.UserRing) myObject;
        UserRing api = (UserRing) apiObject;
        // Ideally we would use the enum name here, my.getType().getName(),
        // but we can't do that yet.  See XCF-794.
        if (properties.contains(TYPE_PROP)) {
            api.setType(myTypeToApiType_(my.getType()));
        }
    }

    private String myTypeToApiType_(Type type) {
        if (type.equals(Type.DELAYED)) {
            return TYPE_DELAYED;
        } else if (type.equals(Type.IMMEDIATE)) {
            return TYPE_IMMEDIATE;
        } else {
            // if we ever get here, then this is a coding error -- should handle all enum values
            throw new RuntimeException("unexpected ring type: " + type.getName());
        }
    }

    public void toMyObject(Object myObject, Object apiObject, Set properties) {
        super.toMyObject(myObject, apiObject, properties);
        org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing my =
            (org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing) myObject;
        UserRing api = (UserRing) apiObject;
        if (properties.contains(TYPE_PROP)) {
            // Ideally we would just retrieve the enum value by name, e.g.
            //     org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing.Type.getEnum(api.getType())
            // but we can't do that yet.  See XCF-794.
            my.setType(apiTypeToMyType_(api.getType()));
        }
    }

    private Type apiTypeToMyType_(String type) {
        if (type.equals(TYPE_DELAYED)) {
            return Type.DELAYED;
        } else if (type.equals(TYPE_IMMEDIATE)) {
            return Type.IMMEDIATE;
        } else {
            // if we ever get here, then this is a coding error -- should handle all enum values
            throw new RuntimeException("unknown ring type: " + type);
        }
    }
}
