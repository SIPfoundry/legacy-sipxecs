/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.Map;

public class SettingMap extends LinkedHashMap<String, Setting> {

    public void acceptVisitor(SettingVisitor visitor) {
        for (Setting setting : values()) {
            setting.acceptVisitor(visitor);
        }
    }

    public Setting addSetting(Setting setting) {
        Setting existingChild = put(setting.getName(), setting);
        if (existingChild != null) {
            Collection<Setting> grandChildren = existingChild.getValues();
            for (Setting grandChild : grandChildren) {
                setting.addSetting(grandChild);
            }
        }

        return setting;
    }

    /**
     * Deep copy or settings kept in setting map
     */
    public SettingMap copy() {
        SettingMap copy = new SettingMap();
        for (Map.Entry<String, Setting> entry : entrySet()) {
            copy.put(entry.getKey(), entry.getValue().copy());
        }
        return copy;
    }
}
