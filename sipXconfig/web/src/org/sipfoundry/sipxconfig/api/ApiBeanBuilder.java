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

import java.util.Set;

/**
 * Transforming beans used in API to beans used in other systems
 */
public interface ApiBeanBuilder {

    public void toApiObject(Object apiObject, Object myObject, Set properties);

    public void toMyObject(Object myObject, Object apiObject, Set properties);
}
