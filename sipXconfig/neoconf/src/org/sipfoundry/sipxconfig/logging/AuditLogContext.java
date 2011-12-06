/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.logging;

import java.util.Set;

import org.sipfoundry.sipxconfig.commserver.Location;

public interface AuditLogContext {
    enum CONFIG_CHANGE_TYPE {
        ADDED, DELETED, MODIFIED;

        @Override
        public String toString() {
            String s = super.toString();
            return s.substring(0, 1) + s.substring(1).toLowerCase();
        }
    }

    enum PROCESS_STATE_CHANGE {
        STOPPED, STARTED, RESTARTED, RELOADED;

        @Override
        public String toString() {
            String s = super.toString();
            return s.substring(0, 1) + s.substring(1).toLowerCase();
        }
    }

    void logReplicationMongo(String dataName, Location location);

    void logReplication(String dataName, Location location);

    void logReplicationFailed(String dataName, Location location, Exception ex);

    void logReplicationMongoFailed(String dataName, Location location, Exception ex);

    void logConfigChange(CONFIG_CHANGE_TYPE changeType, String configType, String configName);

    void logProcessStateChange(PROCESS_STATE_CHANGE stateChange, String processName, Location location);

    Set<String> getReplicationFailedList(String fqdn);

    Set<String> getReplicationSuccededList(String fqdn);

    void resetReplications(String fqdn);

    void saveLatestReplicationsState(Location location);

    boolean isSendProfilesInProgress(Location location);

    public int getProgressPercent(Location location);
}
