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

public interface DaoEventPublisher {
    /**
     * Call this to notify listeners that entity is about to be deleted
     */
    public void publishDelete(Object entity);

    /**
     * Call this to notify listeners that entity is about to be saved or updated
     */
    public void publishSave(Object entity);

}
