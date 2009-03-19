/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.Collection;

import org.sipfoundry.sipxconfig.service.LoggingEntity;

public interface LoggingManager {
    Collection<LoggingEntity> getLoggingEntities();
}
