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

import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.springframework.context.MessageSource;

/**
 * Base class for all items describing and using setting.
 */
public interface Setting extends Cloneable {
    char PATH_DELIM = '/';

    Setting getParent();

    void setParent(Setting parent);

    String getPath();

    Setting addSetting(Setting setting);

    Setting getSetting(String name);

    @Deprecated
    String getLabel();

    String getLabelKey();

    String getName();

    void setName(String name);

    String getProfileName();

    /**
     * Full profile path of the setting - including profile names of all parents and profile name
     * of this setting
     */
    String getProfilePath();

    /**
     * what would value be if it wasn't set., most implementation this is your the value from the
     * setting you decorate. NOTE: no setter because this is a "computed" value based on chain of
     * decorated setting values.
     */
    String getDefaultValue();

    String getValue();

    /**
     * @return the value of this setting coerced to the proper type
     */
    Object getTypedValue();

    void setTypedValue(Object value);

    void setValue(String value);

    SettingType getType();

    void setType(SettingType type);

    int getIndex();

    void setIndex(int i);

    @Deprecated
    String getDescription();

    String getDescriptionKey();

    Collection<Setting> getValues();

    void acceptVisitor(SettingVisitor visitor);

    boolean isAdvanced();

    boolean isHidden();

    boolean isLeaf();

    Setting copy();

    MessageSource getMessageSource();

    String getPathItem(boolean useProfile, boolean useIndex);
}
