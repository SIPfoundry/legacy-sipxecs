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
 * Allows you to dynamically come up w/profile anme
 */
public interface ProfileNameHandler {

    /**
     * @return null if you do not handle this setting and wish to use default
     */
    public SettingValue getProfileName(Setting setting);

}
