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

import java.util.LinkedList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.sipfoundry.sipxconfig.setting.type.StringSetting;
import org.springframework.context.MessageSource;

public abstract class AbstractSetting implements Setting, NamedObject {
    public static final Log LOG = LogFactory.getLog(AbstractSetting.class);

    private static final char KEY_SEPARATOR = '.';

    private static final SettingValue NULL = new SettingValueImpl(null);

    private String m_label;
    private SettingType m_type = StringSetting.DEFAULT;
    private String m_name = StringUtils.EMPTY;
    private String m_description;
    private String m_profileName;
    private Setting m_parent;
    private boolean m_advanced;
    private boolean m_hidden;
    private boolean m_enabled = true;
    private SettingValue m_value = NULL;

    private int m_index = -1;

    public AbstractSetting() {
        super();
    }

    public AbstractSetting(String name) {
        setName(name);
    }

    public Object clone() {
        try {
            return super.clone();
        } catch (CloneNotSupportedException e) {
            throw new RuntimeException("Cannot clone setting", e);
        }
    }

    public Setting copy() {
        return (Setting) clone();
    }

    public Setting getParent() {
        return m_parent;
    }

    public void setParent(Setting parent) {
        m_parent = parent;
    }

    public String getPath() {
        return getPath(PATH_DELIM, false, true);
    }

    public String getProfilePath() {
        return getPath(PATH_DELIM, true, false);
    }

    public void acceptVisitor(SettingVisitor visitor) {
        visitor.visitSetting(this);
    }

    /**
     * @return label if set, otherwise return name as label.
     */
    public String getLabel() {
        return m_label != null ? m_label : m_name;
    }

    public void setLabel(String label) {
        m_label = label;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public void setProfileName(String profileName) {
        m_profileName = profileName;
    }

    public String getProfileName() {
        if (m_profileName == null) {
            return getName();
        }
        return m_profileName;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public boolean isAdvanced() {
        return m_advanced;
    }

    public void setAdvanced(boolean advanced) {
        m_advanced = advanced;
    }

    public boolean isHidden() {
        return m_hidden;
    }

    public void setHidden(boolean hidden) {
        m_hidden = hidden;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public int getIndex() {
        return m_index;
    }

    public void setIndex(int index) {
        m_index = index;
    }

    public SettingType getType() {
        return m_type;
    }

    public void setType(SettingType type) {
        m_type = type;
    }

    public void setTypedValue(Object value) {
        setValue(getType().convertToStringValue(value));
    }

    public Object getTypedValue() {
        return getType().convertToTypedValue(getValue());
    }

    public String getValue() {
        return m_value.getValue();
    }

    public void setValue(String value) {
        m_value = new SettingValueImpl(value);
    }

    /**
     * Builds setting path by iterating through the list of parents
     *
     * @param separator string used to separate path components
     * @param addThis if true this setting name will be also added to path
     * @param useProfile if true build path from profile names
     * @return path created by joining components with a separator
     */
    private String getPath(char separator, boolean useProfile, boolean useIndex) {
        List<String> names = new LinkedList<String>();
        names.add(0, getPathItem(useProfile, useIndex));
        for (Setting p = getParent(); p != null && p.getParent() != null; p = p.getParent()) {
            names.add(0, p.getPathItem(useProfile, useIndex));
        }
        return StringUtils.join(names.iterator(), separator);
    }

    public String getPathItem(boolean useProfile, boolean useIndex) {
        String name = useProfile ? getProfileName() : getName();
        int index = getIndex();
        if (!useIndex || index < 0) {
            return name;
        }
        return String.format("%s[%d]", name, index);
    }

    public String getDescriptionKey() {
        return getPath(KEY_SEPARATOR, false, false) + ".description";
    }

    public String getLabelKey() {
        return getPath(KEY_SEPARATOR, false, false) + ".label";
    }

    public MessageSource getMessageSource() {
        for (Setting p = getParent(); p != null; p = p.getParent()) {
            MessageSource messageSource = p.getMessageSource();
            if (messageSource != null) {
                return messageSource;
            }
        }
        return null;
    }

    /**
     * Find setting coresponding to the path.
     *
     * Please note that setting == setting.getPath("") to support path round-tripping
     *
     * <code>
     *  String s = root.getSetting("x").getParent().getPath();
     *  Setting root = root.getSetting(s);
     * </code>
     *
     * @param path relative to this setting
     * @return found setting or nothing
     *
     */
    public Setting getSetting(String path) {
        if (StringUtils.isEmpty(path)) {
            return this;
        }

        String prefix = path;
        String remainder = null;

        int splitPos = path.indexOf(Setting.PATH_DELIM);
        if (splitPos > 0) {
            prefix = path.substring(0, splitPos);
            // strip '/'
            remainder = path.substring(splitPos + 1);
        }
        Setting child = findChild(prefix);
        if (child == null) {
            // TODO: should we throw an exception here?
            LOG.warn("Cannot find setting: " + path + " in " + this.getPath());
            return null;
        }

        if (remainder == null) {
            // nothing more to do
            return child;
        }

        return child.getSetting(remainder);
    }

    public boolean isLeaf() {
        return getValues().isEmpty();
    }

    protected abstract Setting findChild(String name);
}
