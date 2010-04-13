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

import java.io.Serializable;
import java.util.Collection;
import java.util.Iterator;

import org.springframework.context.MessageSource;

/**
 * Meta information about a group of settings, can contain nested SettingModels. Order is
 * preserved
 */
public class SettingSet extends AbstractSetting implements Serializable {

    private SettingMap m_children = new SettingMap();

    private transient MessageSource m_messageSource;

    /**
     * Root setting group and bean access only
     */
    public SettingSet() {
    }

    public SettingSet(String name) {
        super(name);
    }

    /**
     * includes deep copy of all childen
     */
    @Override
    public Setting copy() {
        SettingSet copy = (SettingSet) shallowCopy();
        for (Setting child : m_children.values()) {
            copy.addSetting(child.copy());
        }
        return copy;
    }

    protected Setting shallowCopy() {
        SettingSet copy = (SettingSet) super.copy();
        copy.m_children = new SettingMap();
        return copy;
    }

    @Override
    public void acceptVisitor(SettingVisitor visitor) {
        if (visitor.visitSettingGroup(this)) {
            m_children.acceptVisitor(visitor);
        }
    }

    /**
     * adds the setting to this group collection along with setting the group on the setting
     */
    public Setting addSetting(Setting setting) {
        setting.setParent(this);
        return m_children.addSetting(setting);
    }

    @Override
    protected Setting findChild(String name) {
        Setting child = m_children.get(name);
        return child;
    }

    public Collection<Setting> getValues() {
        return m_children.values();
    }

    /**
     * @param requiredType returned Setting must be
     * @return default setting in the set - usually first child
     */
    public Setting getDefaultSetting(Class requiredType) {
        for (Iterator i = getValues().iterator(); i.hasNext();) {
            Setting setting = (Setting) i.next();
            if (requiredType.isAssignableFrom(setting.getClass())) {
                return setting;
            }
        }
        return null;
    }

    public String getDefaultValue() {
        throw new UnsupportedOperationException(getPath());
    }

    @Override
    public String getValue() {
        throw new UnsupportedOperationException(getPath());
    }

    @Override
    public void setValue(String value) {
        throw new UnsupportedOperationException(getPath());
    }

    public void setMessageSource(MessageSource messageSource) {
        m_messageSource = messageSource;
    }

    @Override
    public MessageSource getMessageSource() {
        if (m_messageSource != null) {
            return m_messageSource;
        }
        return super.getMessageSource();
    }
}
