/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;

public class ConflictingFeatureCodeValidator extends AbstractSettingVisitor {
    private List<Setting> m_codes;

    /**
     * Reentrant but not multi-threaded
     */
    public void validate(Setting settings) {
        m_codes = new ArrayList<Setting>();
        settings.acceptVisitor(this);
    }

    public void validate(Collection<Setting> settings) {
        m_codes = new ArrayList<Setting>();

        for (Setting setting : settings) {
            setting.acceptVisitor(this);
        }
    }

    public void visitSetting(Setting setting) {
        String value = setting.getValue();
        if (StringUtils.isBlank(value)) {
            return;
        }
        String name = setting.getName();
        if (name.endsWith("_CODE") || name.endsWith("_PREFIX")) {
            for (Setting code : m_codes) {
                String codeValue = code.getValue();
                if (value.startsWith(codeValue) || codeValue.startsWith(value)) {
                    throw new ConflictingFeatureCodeException(setting, code);
                }
            }
            m_codes.add(setting);
        }
    }
}
