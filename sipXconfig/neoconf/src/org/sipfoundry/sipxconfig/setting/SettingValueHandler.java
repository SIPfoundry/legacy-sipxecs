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

/**
 * Resolving a setting value.
 */
public interface SettingValueHandler {

    /**
     * call value.setValue() if implementation has a value for given setting
     */
    public SettingValue getSettingValue(Setting setting);

}
