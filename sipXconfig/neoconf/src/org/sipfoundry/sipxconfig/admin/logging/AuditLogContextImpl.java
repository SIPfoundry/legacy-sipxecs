/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.logging;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.MDC;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class AuditLogContextImpl implements AuditLogContext {

    private static final String REPLICATION_EVENT = "Replication";

    private static final Log AUDIT_LOG = LogFactory.getLog("org.sipfoundry.sipxconfig.auditlog");

    @Override
    public void logReplication(String dataName, Location location) {
        MDC.put("eventType", REPLICATION_EVENT);
        log("Replicated " + dataName + " to " + location.getFqdn());
    }

    private void log(String message) {
        AUDIT_LOG.info(message);
    }
}
