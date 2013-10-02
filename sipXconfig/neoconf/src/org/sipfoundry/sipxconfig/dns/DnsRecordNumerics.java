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
package org.sipfoundry.sipxconfig.dns;

public class DnsRecordNumerics {
    /**
     * Highest priority is the lowest possible number.
     */
    public static final int HIGHEST_PRIORITY = 10;

    /**
     * When there's only one record in a priority range, percentage
     * has no significance.
     */
    public static final int INCONSEQUENTIAL_PERCENTAGE = 10;

    private int m_groupLevel;
    private int m_percentage;

    public DnsRecordNumerics(int groupLevel, int percentage) {
        m_groupLevel = groupLevel;
        m_percentage = percentage;
    }

    public int getPriorityLevel() {
        return m_groupLevel;
    }

    public void setPriorityLevel(int priorityLevel) {
        m_groupLevel = priorityLevel;
    }

    public int getPercentage() {
        return m_percentage;
    }

    public void setPercentage(int percentage) {
        m_percentage = percentage;
    }

    /**
     * groupLevel to priority calculation
     *   0 = 20
     *   1 = 30
     *   2 = 40
     *   ...
     *
     *   numbers themselves are arbitrary (20 v.s. 2), it's more the values when
     *   you juxtaposition them.  Picking orders of 10 might allow for easier
     *   customization down the line.
     */
    public int getPriority() {
        return (m_groupLevel + 2) * 10;
    }

    public static int getHighestPriority() {
        return 10;
    }

    public int getWeight() {
        return m_percentage;
    }
}
