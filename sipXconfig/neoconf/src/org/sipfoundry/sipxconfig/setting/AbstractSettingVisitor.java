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

public class AbstractSettingVisitor implements SettingVisitor {

    public void visitSetting(Setting setting_) {
        // do nothing
    }

    public boolean visitSettingGroup(SettingSet group_) {
        return true;
    }

    public boolean visitSettingArray(SettingArray array) {
        return true;
    }
}
