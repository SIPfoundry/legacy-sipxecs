/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.commons.hz;

import java.io.Serializable;

public abstract class HzMediaEvent implements Serializable {
    private static final long serialVersionUID = 1L;

    private String m_userIdFrom;
    private String m_userIdTo;
    private String m_description;
    private Type m_type;

    public interface Type {
    }

    public HzMediaEvent(String userIdFrom, String userIdTo, String description, Type type) {
        m_userIdFrom = userIdFrom;
        m_userIdTo = userIdTo;
        m_description = description;
        m_type = type;
    }

    public String getUserIdFrom() {
        return m_userIdFrom;
    }

    public String getUserIdTo() {
        return m_userIdTo;
    }

    public String getDescription() {
        return m_description;
    }

    public Type getType() {
        return m_type;
    }
}
