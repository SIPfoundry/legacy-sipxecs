/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.admin;

import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingArray;
import org.sipfoundry.sipxconfig.setting.SettingSet;
/**
 * By default, the blank policy is used and *custom* policy is not activated
 * @see PasswordPolicy interface on how to implement *custom* policy
 */
public class PasswordPolicyImpl implements PasswordPolicy {

    @Override
    public void visitSetting(Setting setting) {
    }

    @Override
    public boolean visitSettingGroup(SettingSet group) {
        return false;
    }

    @Override
    public boolean visitSettingArray(SettingArray array) {
        return false;
    }

    @Override
    public String getDefaultPolicy() {
        return AdminContext.PasswordPolicyType.blank.name();
    }

    @Override
    public String getPassword() {
        return null;
    }

    @Override
    public String getVoicemailPin() {
        return null;
    }

}
