/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.attendant;

import java.util.LinkedList;
import java.util.List;

import org.sipfoundry.sipxivr.ApplicationConfiguraton;

public class AttendantConfig extends ApplicationConfiguraton {
    private String m_id; // The ID of this attendant
    private String m_name; // The name of this attendant
    private String m_prompt; // The top level prompt
    private List<AttendantMenuItem> m_menuItems = new LinkedList<AttendantMenuItem>();;

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
        m_menuItems.add(item);
    }

    public List<AttendantMenuItem> getMenuItems() {
        return m_menuItems;
    }

}
