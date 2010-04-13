/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import java.util.Iterator;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.LocalizationUtils;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingArray;
import org.springframework.context.MessageSource;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class SettingsGrid extends BaseComponent {
    @Parameter(required = true)
    public abstract SettingArray getSetting();

    @Parameter(required = true)
    public abstract boolean isRequiredEnabled();

    @Parameter(required = true)
    public abstract boolean isEnabled();

    @Parameter(required = true)
    public abstract MessageSource getMessageSource();

    public abstract String getCurrentColumn();

    public abstract int getCurrentIndex();

    public Setting getCurrentSetting() {
        int index = getCurrentIndex();
        String column = getCurrentColumn();
        return getSetting().getSetting(index, column);
    }

    public String getCurrentColumnLabel() {
        String column = getCurrentColumn();
        Setting setting = getSetting().getSetting(0, column);
        return LocalizationUtils.getSettingLabel(this, setting);
    }

    public String getDescription() {
        return LocalizationUtils.getSettingDescription(this, getSetting());
    }

    public Iterator<Integer> getIndices() {
        return new IndexIterator(getSetting().getSize());
    }

    static class IndexIterator implements Iterator<Integer> {
        private final int m_size;
        private int m_next;

        public IndexIterator(int size) {
            m_size = size;
        }

        public boolean hasNext() {
            return m_next < m_size;
        }

        public Integer next() {
            return m_next++;
        }

        public void remove() {
            throw new UnsupportedOperationException();
        }
    }
}
