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



public interface SettingFilter {

    /**
     * Effectively returns all settings and setting groups recursively
     * not including root setting.
     */
    public static final SettingFilter ALL = new SettingFilter() {
        public boolean acceptSetting(Setting root_, Setting setting_) {
            return true;
        }
    };

    public boolean acceptSetting(Setting root, Setting setting);
}
