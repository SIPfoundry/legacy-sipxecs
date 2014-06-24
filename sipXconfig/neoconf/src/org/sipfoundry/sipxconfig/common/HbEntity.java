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
package org.sipfoundry.sipxconfig.common;

import java.io.Serializable;

import org.hibernate.type.Type;

public class HbEntity {
    private Object m_entity;
    private Serializable m_id;
    private Object[] m_newValues;
    private Object[] m_oldValues;
    private String[] m_properties;
    private Type[] m_types;
    private Object[] m_state;

    public HbEntity(Object entity, Serializable id, Object[] newValues, Object[] oldValues, String[] properties,
        Type[] types, Object[] state) {
        m_entity = entity;
        m_id = id;
        m_newValues = newValues;
        m_oldValues = oldValues;
        m_properties = properties;
        m_types = types;
        m_state = state;
    }
    public Object getEntity() {
        return m_entity;
    }
    public Serializable getId() {
        return m_id;
    }
    public Object[] getNewValues() {
        return m_newValues;
    }
    public Object[] getOldValues() {
        return m_oldValues;
    }
    public String[] getProperties() {
        return m_properties;
    }
    public Type[] getTypes() {
        return m_types;
    }
    public Object[] getState() {
        return m_state;
    }


}
