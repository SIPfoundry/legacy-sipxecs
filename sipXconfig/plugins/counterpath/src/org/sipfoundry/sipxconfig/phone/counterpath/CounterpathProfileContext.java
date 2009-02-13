/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.counterpath;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;

public class CounterpathProfileContext extends ProfileContext<Phone> {
    public CounterpathProfileContext(Phone device, String profileTemplate) {
        super(device, profileTemplate);
    }

    @Override
    public Map<String, Object> getContext() {
        Map<String, Object> context = super.getContext();
        LeafSettings leafs = new LeafSettings();
        List<Collection> lineLeafSettings = new ArrayList<Collection>();
        Phone phone = getDevice();
        List<Line> lines = phone.getLines();
        phone.getSettings().acceptVisitor(leafs);
        context.put("phone_leaf_settings", leafs.getLeafs());
        for (Line line : lines) {
            leafs = new LeafSettings();
            line.getSettings().acceptVisitor(leafs);
            lineLeafSettings.add(leafs.getLeafs());
        }
        context.put("line_leaf_settings", lineLeafSettings);
        return context;
    }

    private class LeafSettings extends AbstractSettingVisitor {

        private final Collection<Setting> m_leaf = new ArrayList<Setting>();

        public Collection<Setting> getLeafs() {
            return m_leaf;
        }

        @Override
        public void visitSetting(Setting setting) {
            if (setting.isLeaf()) {
                m_leaf.add(setting);
            }
        }
    }
}
