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

import java.util.HashSet;
import java.util.Set;

/**
 * Copies all getters from one bean into all the available, matching
 * setters of another bean.
 */
public class SimpleBeanBuilder implements ApiBeanBuilder {
    public static final String ID_PROP = "id";

    private Set m_customFields;

    public Set getCustomFields() {
        if (m_customFields == null) {
            m_customFields = new HashSet();
        }

        return m_customFields;
    }

    public void toApiObject(Object apiObject, Object myObject, Set properties) {
        ApiBeanUtil.copyProperties(apiObject, myObject, properties, m_customFields);
    }

    public void toMyObject(Object myObject, Object apiObject, Set properties) {
        ApiBeanUtil.copyProperties(myObject, apiObject, properties, m_customFields);
    }
}
