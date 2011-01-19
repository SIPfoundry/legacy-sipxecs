/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.openacd;

import java.util.LinkedList;
import java.util.List;

public class FreeswitchMediaCommand extends OpenAcdConfigObject {

    private boolean m_enabled;
    private String m_node;
    private String m_dialString;

    public FreeswitchMediaCommand(boolean enabled, String node, String dialString) {
        m_enabled = enabled;
        m_node = node;
        m_dialString = dialString;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public void setNode(String node) {
        m_node = node;
    }

    public String getNode() {
        return m_node;
    }

    public void setDialString(String dialString) {
        m_dialString = dialString;
    }

    public String getDialString() {
        return m_dialString;
    }

    @Override
    public List<String> getProperties() {
        List<String> props = new LinkedList<String>();
        props.add("enabled");
        props.add("node");
        props.add("dialString");
        return props;
    }

    @Override
    public String getType() {
        return "freeswitch_media_manager";
    }
}
