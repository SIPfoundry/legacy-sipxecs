/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.attendant;

import java.util.LinkedList;
import java.util.List;

import org.sipfoundry.sipxivr.ApplicationConfiguraton;

public class AttendantConfig extends ApplicationConfiguraton {
    private String m_id; // The ID of this attendant
    private String m_name; // The name of this attendant
    private String m_prompt; // The top level prompt
    private List<AttendantMenuItem> m_menuItems;

    public AttendantConfig() {
        super();
    }

    public String getId() {
        return m_id;
    }

    public void setId(String id) {
        m_id = id;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getPrompt() {
        return m_prompt;
    }

    public void setPrompt(String prompt) {
        m_prompt = prompt;
    }

    public void addMenuItem(AttendantMenuItem item) {
        if (m_menuItems == null) {
            m_menuItems = new LinkedList<AttendantMenuItem>();
        }
        m_menuItems.add(item);
    }

    public List<AttendantMenuItem> getMenuItems() {
        return m_menuItems;
    }

}
