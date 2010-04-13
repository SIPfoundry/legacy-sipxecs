/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import java.util.Hashtable;

public interface Provisioning {
    public static final Integer SUCCESS = new Integer(1);

    Hashtable create(Hashtable params);

    Hashtable delete(Hashtable params);

    Hashtable set(Hashtable params);

    Hashtable get(Hashtable params);
}
