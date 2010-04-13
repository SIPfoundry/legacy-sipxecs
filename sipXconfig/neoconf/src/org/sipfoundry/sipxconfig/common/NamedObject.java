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

import org.apache.commons.collections.Transformer;

public interface NamedObject {
    public String getName();

    public void setName(String name);

    public static class ToName implements Transformer {
        public Object transform(Object input) {
            return ((NamedObject) input).getName();
        }
    }
}
