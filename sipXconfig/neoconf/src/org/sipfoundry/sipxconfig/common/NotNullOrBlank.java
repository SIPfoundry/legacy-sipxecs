/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.apache.commons.lang.StringUtils;
import org.hibernate.criterion.Example.PropertySelector;
import org.hibernate.type.Type;

public final class NotNullOrBlank implements PropertySelector {
    public static final PropertySelector INSTANCE = new NotNullOrBlank();

    private NotNullOrBlank() {
        // use single static instance
    }

    public boolean include(Object propertyValue, String propertyName_, Type type_) {
        if (propertyValue == null) {
            return false;
        }
        if (propertyValue instanceof String) {
            return StringUtils.isNotBlank((String) propertyValue);
        }
        return true;
    }
}
