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

import java.util.Collection;

public interface DaoEventPublisher {

    /**
     * Call this to notify listeners that every entity in this collection is about to be deleted
     */
    public void publishDeleteCollection(Collection<?> entities);

    /**
     * Call this to notify listeners that every entity in the collection was saved or updated
     */
    public void publishSaveCollection(Collection<?> entities);

    /**
     * Call this to notify listeners that every entity in the collection was saved or updated
     */
    public void publishBeforeSaveCollection(Collection<?> entities);

    /**
     * Call this to notify listeners that every entity in the collection was saved or updated
     */
    public void publishAfterDeleteCollection(Collection<?> entities);

    /**
     * Call this to notify listeners that entity is about to be deleted
     */
    public void publishDelete(Object entity);

    /**
     * Call this to notify listeners that entity was saved or updated
     */
    public void publishSave(Object entity);

    /**
     * Call this to notify listeners that entity was saved or updated
     */
    public void publishBeforeSave(Object entity);

    /**
     * Call this to notify listeners that entity was saved or updated
     */
    public void publishAfterDelete(Object entity);
}
