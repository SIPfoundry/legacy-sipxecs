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
import java.util.List;

import org.sipfoundry.sipxconfig.service.LoggingEntity;
import org.sipfoundry.sipxconfig.service.SipxService;

public interface LoggingManager {
    Collection<LoggingEntity> getLoggingEntities();
    List<LoggingEntity> getEntitiesToProcess();
    void setEntitiesToProcess(List<LoggingEntity> entitiesToProcess);
    SipxService getSipxServiceForLoggingEntity(LoggingEntity loggingEntity);
}
