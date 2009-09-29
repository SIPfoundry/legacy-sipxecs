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

import java.io.Serializable;

/**
 * Support common data object marshalling
 */
public interface DataObjectSource<T> {

    /** Read object from data source by class and object id */
    public T load(Class<T> c, Serializable serializable);
}
