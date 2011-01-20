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

public class OpenAcdAgentConfigCommand extends OpenAcdConfigObject {

    private boolean m_listenerEnabled;

    public OpenAcdAgentConfigCommand(boolean enabled) {
        m_listenerEnabled = enabled;
    }

    public boolean isListenerEnabled() {
        return m_listenerEnabled;
    }

    public void setListenerEnabled(boolean enabled) {
        m_listenerEnabled = enabled;
    }

    @Override
    public List<String> getProperties() {
        List<String> props = new LinkedList<String>();
        props.add("listenerEnabled");
        return props;
    }

    @Override
    public String getType() {
        return "agent_configuration";
    }
}
