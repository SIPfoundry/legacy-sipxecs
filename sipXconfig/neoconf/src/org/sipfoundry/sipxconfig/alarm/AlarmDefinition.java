/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.alarm;

public class AlarmDefinition {
    private String m_id;
    private int m_defaultMinimumThreshold;

    public AlarmDefinition(String id) {
        m_id = id;
    }

    public AlarmDefinition(String id, int defaultMinimumThreshold) {
        this(id);
        m_defaultMinimumThreshold = defaultMinimumThreshold;
    }

    public String getId() {
        return m_id;
    }

    public int getDefaultMinimumThreshold() {
        return m_defaultMinimumThreshold;
    }
}
