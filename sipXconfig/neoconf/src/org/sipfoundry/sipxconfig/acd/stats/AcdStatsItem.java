/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd.stats;

/**
 * Allow polymorphic access to data across independant statistical subclasses.
 */
public class AcdStatsItem {

    public static interface AgentName {
        public String getAgentName();
    }

    public static interface QueueName {
        public String getQueueName();
    }
}
