/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.mongo;

import java.util.Collections;
import java.util.List;

public class MongoNode {
    private static final List<String> NONE = Collections.emptyList();
    private String m_hostPort;
    private List<String> m_status;

    public MongoNode(String hostPort, List<String> status) {
        m_hostPort = hostPort;
        m_status = status;
    }

    public String getHostPort() {
        return m_hostPort;
    }

    public List<String> getStatus() {
        return m_status == null ? NONE : m_status;
    }

    public String getFqdn() {
        return fqdn(m_hostPort);
    }

    public boolean isArbiter() {
        return m_hostPort.endsWith(String.valueOf(MongoSettings.ARBITER_PORT));
    }

    public static String fqdn(String hostPort) {
        int colon = hostPort.indexOf(':');
        return colon > 0 ? hostPort.substring(0, colon) : hostPort;
    }

    public static String arbiterHostPort(String fqdn) {
        return fqdn + ':' + MongoSettings.ARBITER_PORT;
    }

    public static String databaseHostPort(String fqdn) {
        return fqdn + ':' + MongoSettings.SERVER_PORT;
    }
}
