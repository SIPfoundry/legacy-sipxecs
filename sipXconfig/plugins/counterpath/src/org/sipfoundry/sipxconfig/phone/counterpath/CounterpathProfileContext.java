/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phone.counterpath;

import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.MultiEnumSetting;

public class CounterpathProfileContext extends ProfileContext<Phone> {
    public CounterpathProfileContext(Phone device, String profileTemplate) {
        super(device, profileTemplate);
    }

    @Override
    public Map<String, Object> getContext() {
        Map<String, Object> context = super.getContext();
        Phone phone = getDevice();
        LeafSettings ps = new LeafSettings();
        phone.getSettings().acceptVisitor(ps);
        context.put("phone_leaf_settings", ps.getSip());

        List<Line> lines = phone.getLines();
        List<Collection> lineSipSettings = new ArrayList<Collection>();
        List<Collection> lineXmppSettings = new ArrayList<Collection>();
        for (Line line : lines) {
            LeafSettings ls = new LeafSettings();
            line.getSettings().acceptVisitor(ls);
            lineSipSettings.add(ls.getSip());
            if (isImEnabled(line)) {
                lineXmppSettings.add(ls.getXmpp());
            }
        }
        context.put("line_sip_settings", lineSipSettings);
        context.put("line_xmpp_settings", lineXmppSettings);
        context.put("max_lines", getDevice().getModel().getMaxLineCount());

        context.put("priorityCalculator", new PriorityCalculator());

        return context;
    }

    private boolean isImEnabled(Line line) {
        User user = line.getUser();
        if (user == null) {
            return false;
        }
        return new ImAccount(user).isEnabled();
    }

    /**
     * Separates XMPP and SIP settings
     */
    private static class LeafSettings extends AbstractSettingVisitor {

        private final Collection<Setting> m_sip = new ArrayList<Setting>();
        private final Collection<Setting> m_xmpp = new ArrayList<Setting>();

        public Collection<Setting> getSip() {
            return m_sip;
        }

        public Collection<Setting> getXmpp() {
            return m_xmpp;
        }

        @Override
        public void visitSetting(Setting setting) {
            if (!setting.isLeaf()) {
                return;
            }
            Collection<Setting> leaves = isXmpp(setting) ? m_xmpp : m_sip;
            leaves.add(setting);
        }

        private boolean isXmpp(Setting setting) {
            return "xmpp-config".equals(setting.getParent().getProfileName());
        }
    }

    public class PriorityCalculator {
        public Map<String, String> getCodecPriorities(Setting setting) {
            Map<String, String> priorities = new LinkedHashMap<String, String>();
            List<String> values = (List<String>) setting.getTypedValue();
            for (String codec : getAllCodecs(setting)) {
                priorities.put(codec, StringUtils.EMPTY);
            }
            int priority = 1;
            for (String value : values) {
                priorities.put(value, (priority++) + ".0");
            }
            return priorities;
        }

        private Set<String> getAllCodecs(Setting codecs) {
            MultiEnumSetting type = (MultiEnumSetting) codecs.getType();
            Map<String, String> allCodecs = type.getEnums();
            return allCodecs.keySet();
        }
    }

}
