/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.setting;

/**
 * Use to adjust settings values: to be used with DelegatingSettingModel
 */
public interface SettingValueFilter {
    SettingValue filter(SettingValue sv);
}
