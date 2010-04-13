/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting.type;

public interface SettingType extends Cloneable {
    String getName();

    boolean isRequired();

    /**
     * Converts between internal representation of the value (usually String) and external
     * representation determined by this type.
     *
     * @param value internal representation of the setting value
     * @return the same value coerced to typed value Integer, Boolean etc.
     */
    Object convertToTypedValue(Object value);

    String convertToStringValue(Object value);

    /**
     * Return human representations of value, not internal reprentations of value,
     * not always the same, for example boolean or list types
     */
    String getLabel(Object value);

    void setId(String id);

    public SettingType clone();
}
