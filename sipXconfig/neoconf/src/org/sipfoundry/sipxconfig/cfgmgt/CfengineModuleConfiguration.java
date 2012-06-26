/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.cfgmgt;

import java.io.IOException;
import java.io.Writer;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;
import org.sipfoundry.sipxconfig.setting.type.MultiEnumSetting;

public class CfengineModuleConfiguration extends AbstractConfigurationFile {
    private static final String EOL = System.getProperty("line.separator");
    private Writer m_out;

    public CfengineModuleConfiguration(Writer w) {
        m_out = w;
    }

    public void writeClass(String property, boolean enabled) throws IOException {
        m_out.write(enabled ? '+' : '-');
        m_out.write(property);
        m_out.write(EOL);
    }

    public void writeSetting(String prefix, Setting setting) throws IOException {
        if (setting.getType() instanceof BooleanSetting) {
            m_out.write(Boolean.TRUE.equals(setting.getTypedValue()) ? '+' : '-');
            if (prefix != null) {
                m_out.write(prefix);
            }
            m_out.write(setting.getName());
            m_out.write(EOL);
        } else if (setting.getType() instanceof MultiEnumSetting) {
            List<?> values = (List<?>) setting.getTypedValue();
            String key = setting.getName();
            if (prefix != null) {
                key = prefix + key;
            }
            writeList(key, values);
        } else {
            write(prefix, setting.getName(), setting.getValue());
        }
    }

    @Override
    public void write(String prefix, String key, Object value) throws IOException {
        m_out.write('=');
        if (prefix != null) {
            m_out.write(prefix);
        }
        m_out.write(key);
        m_out.write('=');
        m_out.write(safeToString(value));
        m_out.write(EOL);
    }

    public void writeList(String key, Collection< ? > items) throws IOException {
        if (items.isEmpty()) {
            // cannot seem to be able to define empty list
            return;
        }
        m_out.write('@');
        m_out.write(key);
        m_out.write("={\"");
        Iterator< ? > i = items.iterator();
        for (int ndx = 0; i.hasNext(); ndx++) {
            if (ndx != 0) {
                m_out.write("\", \"");
            }
            m_out.write(safeToString(i.next()));

        }
        m_out.write("\"}");
        m_out.write(EOL);
    }

    String safeToString(Object value) {
        return value == null ? "" : value.toString();
    }
}
