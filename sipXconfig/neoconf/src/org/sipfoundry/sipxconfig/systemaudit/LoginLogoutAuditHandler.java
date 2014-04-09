/**
 *
 * Copyright (c) 2013 Karel Electronics Corp. All rights reserved.
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
 *
 */

package org.sipfoundry.sipxconfig.systemaudit;

import javax.servlet.http.HttpSession;

import org.apache.commons.beanutils.BeanUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.context.ApplicationEvent;
import org.springframework.security.authentication.event.AbstractAuthenticationFailureEvent;
import org.springframework.security.authentication.event.InteractiveAuthenticationSuccessEvent;
import org.springframework.security.core.Authentication;
import org.springframework.security.web.session.HttpSessionDestroyedEvent;

/**
 * This class contains the business logic of Login/Logout actions
 */
public class LoginLogoutAuditHandler extends AbstractSystemAuditHandler {

    private static final String SUCCESS = "Success";
    private static final String FAILED = "Failed";

    public void handleLoginLogoutConfigChange(ApplicationEvent appEvent) throws Exception {
        if (appEvent instanceof InteractiveAuthenticationSuccessEvent) {
            loginSuccess((InteractiveAuthenticationSuccessEvent) appEvent);
        } else if (appEvent instanceof AbstractAuthenticationFailureEvent) {
            loginFailed((AbstractAuthenticationFailureEvent) appEvent);
        } else if (appEvent instanceof HttpSessionDestroyedEvent) {
            logout((HttpSessionDestroyedEvent) appEvent);
        }
    }

    private void loginSuccess(InteractiveAuthenticationSuccessEvent loginSuccessEvent)
        throws SystemAuditException {
        ConfigChange configChange = buildConfigChange(ConfigChangeAction.LOGIN, ConfigChangeType.LOGIN_LOGOUT);
        configChange.setDetails(SUCCESS);
        getConfigChangeContext().storeConfigChange(configChange);
    }

    private void loginFailed(AbstractAuthenticationFailureEvent loginFailedEvent) throws SystemAuditException {
        Authentication authentication = loginFailedEvent.getAuthentication();
        String userName = (String) authentication.getPrincipal();
        ConfigChange configChange = buildConfigChange(ConfigChangeAction.LOGIN,
                ConfigChangeType.LOGIN_LOGOUT, userName, AbstractSystemAuditHandler.LOCALHOST);
        configChange.setDetails(FAILED);
        getConfigChangeContext().storeConfigChange(configChange);
    }

    private void logout(HttpSessionDestroyedEvent logoutEvent) throws Exception {
        HttpSession session = logoutEvent.getSession();
        Object userSession = (Object) session.getAttribute("state:sipXconfig-web:userSession");
        String userIdString = BeanUtils.getProperty(userSession, "userId");
        if (userIdString == null || userIdString.isEmpty()) {
            return;
        }
        Integer userId = Integer.parseInt(userIdString);
        User user = getCoreContext().getUser(userId);
        ConfigChange configChange = buildConfigChange(ConfigChangeAction.LOGOUT,
                ConfigChangeType.LOGIN_LOGOUT, user.getName(), AbstractSystemAuditHandler.LOCALHOST);
        configChange.setDetails(SUCCESS);
        getConfigChangeContext().storeConfigChange(configChange);
    }
}
