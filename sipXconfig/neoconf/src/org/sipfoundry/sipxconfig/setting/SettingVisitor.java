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

public interface SettingVisitor {

    public void visitSetting(Setting setting);

    /**
     * @return false if you do not want to visit children
     */
    public boolean visitSettingGroup(SettingSet group);

    /**
     * @return false if you do not want to visit elements
     */
    public boolean visitSettingArray(SettingArray array);
}
