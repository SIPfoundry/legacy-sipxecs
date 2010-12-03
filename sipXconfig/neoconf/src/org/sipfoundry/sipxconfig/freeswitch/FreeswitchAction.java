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
package org.sipfoundry.sipxconfig.freeswitch;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.common.BeanWithId;

public class FreeswitchAction extends BeanWithId implements Serializable {
    private String m_application;
    private String m_data;

    public static enum PredefinedAction {
        answer, set, erlang_sendmsg, log, erlang, playback, sleep, hangup;

        public static List<String> valuesAsStrings() {
            List<String> actions = new ArrayList<String>();
            for (PredefinedAction action : values()) {
                actions.add(action.toString());
            }
            return actions;
        }
    }

    public String getApplication() {
        return m_application;
    }

    public void setApplication(String application) {
        m_application = application;
    }

    public String getData() {
        return m_data;
    }

    public void setData(String data) {
        m_data = data;
    }

    @Override
    public int hashCode() {
        return new HashCodeBuilder().append(m_application).append(m_data).toHashCode();
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof FreeswitchAction)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        FreeswitchAction bean = (FreeswitchAction) other;
        return new EqualsBuilder().append(m_application, bean.m_application).append(m_data, bean.m_data).isEquals();
    }

}
