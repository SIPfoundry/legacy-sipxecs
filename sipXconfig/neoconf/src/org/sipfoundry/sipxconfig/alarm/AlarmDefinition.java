/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.alarm;

import java.util.Arrays;
import java.util.Collection;

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

    public static Collection<AlarmDefinition> asArray(String[] ids) {
        AlarmDefinition[] defs = new AlarmDefinition[ids.length];
        for (int i = 0; i < defs.length; i++) {
            defs[i] = new AlarmDefinition(ids[i]);
        }
        return Arrays.asList(defs);
    }
}
