/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common.event;


/**
 * Any beans that implement this interface will be called when entities are saved
 * or deleted.  Dao's must declare DaoEventDispatcher (or any of it's subclasses)
 * as it's Spring-Hibernate interceptor for events to be sent
 */
public interface DaoEventListener {

    /**
     * Is called before the actual entity is deleted
     */
    public void onDelete(Object entity);

    /**
     * Is called before the actual entity is saved or updated
     */
    public void onSave(Object entity);
}
