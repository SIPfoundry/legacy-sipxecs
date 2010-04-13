/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;


/**
 * This represent the expected response of the auto attendant after user presses a dial-pad key
 */
public class AttendantMenuItem {

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
}
