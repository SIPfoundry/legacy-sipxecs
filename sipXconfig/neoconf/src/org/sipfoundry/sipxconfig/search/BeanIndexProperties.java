/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.search;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.hibernate.type.Type;

public class BeanIndexProperties {
    private Object m_entity;
    private Serializable m_id;
    private List<Object> m_state;
    private List<String> m_propertyNames;
    private List<Type> m_types;

    BeanIndexProperties(Object entity, Serializable id, Object[] state, String[] propertyNames, Type[] types) {
        m_entity = entity;
        m_id = id;
        m_state = new ArrayList<Object>(Arrays.asList(state));
        m_propertyNames = new ArrayList<String>(Arrays.asList(propertyNames));
        m_types = new ArrayList<Type>(Arrays.asList(types));
    }

    public List<Object> getStateList() {
        return m_state;
    }

    public Object[] getState() {
        return m_state.toArray(new Object[0]);
    }

    public void setState(List<Object> state) {
        m_state = state;
    }

    public List<String> getPropertyNamesList() {
        return m_propertyNames;
    }

    public String[] getPropertyNames() {
        return m_propertyNames.toArray(new String[0]);
    }

    public void setPropertyNames(List<String> propertyNames) {
        m_propertyNames = propertyNames;
    }

    public List<Type> getTypesList() {
        return m_types;
    }

    public Type[] getTypes() {
        return m_types.toArray(new Type[0]);
    }

    public void setTypes(List<Type> types) {
        m_types = types;
    }

    public Object getEntity() {
        return m_entity;
    }

    public Serializable getId() {
        return m_id;
    }
}
