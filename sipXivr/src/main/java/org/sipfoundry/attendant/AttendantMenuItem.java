/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.attendant;

public class AttendantMenuItem {

    private String m_dialpad;
    private Actions m_action;
    private String m_parameter;
    private String m_extension;

    public AttendantMenuItem(String dialpad, Actions action, String parameter, String extension) {
        this.m_dialpad = dialpad;
        this.m_action = action;
        this.m_parameter = parameter;
        this.m_extension = extension;
    }

    public String getDialpad() {
        return m_dialpad;
    }

    public void setDialpad(String dialpad) {
        m_dialpad = dialpad;
    }

    public Actions getAction() {
        return m_action;
    }

    public void setAction(Actions action) {
        m_action = action;
    }

    public String getParameter() {
        return m_parameter;
    }

    public void setParameter(String parameter) {
        m_parameter = parameter;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }
    
}
