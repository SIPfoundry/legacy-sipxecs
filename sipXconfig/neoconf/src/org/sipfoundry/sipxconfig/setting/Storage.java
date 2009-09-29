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

public interface Storage extends SettingValueHandler {

    public void setSettingValue(Setting setting, SettingValue value, SettingValue defaultValue);

    public void revertSettingToDefault(Setting setting);
}
