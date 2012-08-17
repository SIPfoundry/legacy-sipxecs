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

import org.sipfoundry.sipxconfig.setting.SettingVisitor;
/**
 * The admin defines three password policies: blank, defaultValue, custom
 * The first two policies are implemented and ready to use: blank and defaultValue
 * @see AdminContext.PasswordPolicyType
 *
 * -blank policy -
 * means that password and voicemail pin are left empty when a new user is about to be created and
 * the administrator needs to fill the values manually
 * -defaultValue policy -
 * means that the password and voicemail pin are filled with default values
 * specified by the addministrator in Settings/Admin Settings page
 * -custom policy is a user defined policy:
 *
 * By implementing this interface, and defining a spring bean
 * in a plugin's sipxplugin.beans.xml that overwrites the actual
 * implementation of PasswordPolicy (PasswordPolicyImpl) the user can add his own strategy to compute passwords
 *
 * Example:
 * public class MyPasswordPolicy implements PasswordPolicy {
 *     //Activate custom policy, make it available for selection in Settings/Admin Settings page
 *     @Override
 *     public void visitSetting(Setting setting) {
 *          SettingType type = setting.getType();
 *          if (type instanceof EnumSetting && setting.getPath().equals("configserver-config/password-policy")) {
 *              EnumSetting policyType = (EnumSetting) type;
 *              policyType.addEnum(AdminContext.PasswordPolicyType.custom.name(), "Custom Password/VM Pin Values");
 *          }
 *     }
 *     //Default policy used is this policy, and not an admin defined policy(blank or default value)
 *     public String getDefaultPolicy() {
 *          return AdminContext.PasswordPolicyType.custom.name();
 *     }
 *     public String getPassword() {
 *          return "define your own rule";
 *     }
 *     public String getVoicemailPin() {
 *          return "define your own rule";
 *     }
 * }
 */
public interface PasswordPolicy extends SettingVisitor {
    String getDefaultPolicy();
    String getPassword();
    String getVoicemailPin();
}
