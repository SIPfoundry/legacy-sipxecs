/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan;

import org.sipfoundry.sipxconfig.common.NamedObject;


/**
 * This represent the expected response of the auto attendant after user presses a dial-pad key
 */
public class AttendantMenuItem implements NamedObject {

    private AttendantMenuAction m_action;

    private String m_parameter;

    /** BEAN ACCESS */
    public AttendantMenuItem() {
    }

    public AttendantMenuItem(AttendantMenuAction action) {
        setAction(action);
    }

    public AttendantMenuItem(AttendantMenuAction action, String parameter) {
        this(action);
        setParameter(parameter);
    }

    public AttendantMenuAction getAction() {
        return m_action;
    }

    public void setAction(AttendantMenuAction action) {
        m_action = action;
    }

    /**
     * Depends on the operation type, but this could be extension or aa menu name
     */
    public String getParameter() {
        return m_parameter;
    }

    public void setParameter(String parameter) {
        m_parameter = parameter;
    }

    @Override
    public String toString() {
        return "AttendantMenuItem [m_action=" + m_action + ", m_parameter=" + m_parameter + "]";
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((m_action == null) ? 0 : m_action.hashCode());
        result = prime * result + ((m_parameter == null) ? 0 : m_parameter.hashCode());
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null) {
            return false;
        }
        if (getClass() != obj.getClass()) {
            return false;
        }
        AttendantMenuItem other = (AttendantMenuItem) obj;
        if (m_action == null) {
            if (other.m_action != null) {
                return false;
            }
        } else if (!m_action.equals(other.m_action)) {
            return false;
        }
        if (m_parameter == null) {
            if (other.m_parameter != null) {
                return false;
            }
        } else if (!m_parameter.equals(other.m_parameter)) {
            return false;
        }
        return true;
    }

    @Override
    public String getName() {
        return getParameter();
    }

    @Override
    public void setName(String name) {
        //Do Nothing
    }
}
