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
package org.sipfoundry.sipxconfig.health;

public final class HealthCheck {
    private boolean m_passed;
    private String m_message;

    private HealthCheck(String message, boolean passed) {
        m_passed = passed;
        m_message = message;
    }

    public static HealthCheck failed(String err) {
        return new HealthCheck(err, false);
    }

    public static final HealthCheck passed(String msg) {
        return new HealthCheck(msg, true);
    }

    public boolean isPassed() {
        return m_passed;
    }

    public String getMessage() {
        return m_message;
    }
}
