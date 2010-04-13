/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.login;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.acegisecurity.event.authentication.AbstractAuthenticationEvent;
import org.acegisecurity.event.authentication.AbstractAuthenticationFailureEvent;
import org.acegisecurity.event.authentication.InteractiveAuthenticationSuccessEvent;
import org.acegisecurity.ui.WebAuthenticationDetails;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.AlarmContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Md5Encoder;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class LoginContextImpl implements LoginContext, ApplicationListener {
    private static final String USERLOGINS_LOG = "sipxconfig-logins.log";

    private static final Log LOG = LogFactory.getLog("login");

    private AlarmContext m_alarmContext;
    private CoreContext m_coreContext;

    private String m_logDirectory;

    /**
     * Returns user if credentials check out. Return null if the user does not exist or the
     * password is wrong. The userId may be either the userName or an alias.
     */
    public User checkCredentials(String userNameOrAlias, String password) {
        User user = m_coreContext.loadUserByUserNameOrAlias(userNameOrAlias);
        if (user == null) {
            return null;
        }

        String userName = user.getUserName();
        String pintoken = user.getPintoken();
        String encodedPassword = getEncodedPassword(userName, password);

        // Real match
        if (encodedPassword.equals(pintoken)) {
            return user;
        }

        // Special case: if the password is empty and the pintoken is empty, then declare a match.
        // We have publicized the ability for admins to reset users to have an empty password by
        // zeroing out the pintoken entry in the database.
        if (StringUtils.isBlank(password) && pintoken.length() == 0) {
            return user;
        }

        return null;
    }

    private void logLoginAttempt(String userNameOrAlias, boolean success, String remoteIp) {
        if (remoteIp == null) {
            return;
        }
        if (success) {
            LOG.info(LoginEvent.formatLogToRecord(LoginEvent.SUCCESS, userNameOrAlias, remoteIp));
        } else {
            LOG.warn(LoginEvent.formatLogToRecord(LoginEvent.FAILURE, userNameOrAlias, remoteIp));
        }
    }

    public String getEncodedPassword(String userName, String password) {
        return Md5Encoder.digestPassword(userName, m_coreContext.getAuthorizationRealm(), password);
    }

    public boolean isAdmin(Integer userId) {
        User user = m_coreContext.loadUser(userId);
        return isAdmin(user);
    }

    public boolean isAdmin(User user) {
        if (user == null) {
            return false;
        }
        return user.isAdmin();
    }

    public LoginEvent[] getUserLoginLog(LogFilter filter) {
        File log = new File(m_logDirectory, USERLOGINS_LOG);
        List<LoginEvent> contents = new ArrayList<LoginEvent>();
        BufferedReader input = null;
        try {
            input = new BufferedReader(new FileReader(log));
            String line = null;
            while ((line = input.readLine()) != null) {
                LoginEvent logEvent = new LoginEvent(line);
                if (filter.isLogEntryInQuery(logEvent)) {
                    contents.add(logEvent);
                }
            }
        } catch (FileNotFoundException ex) {
            throw new UserException("log.notFound");
        } catch (IOException ex) {
            throw new UserException("log.cannotRead");
        } finally {
            IOUtils.closeQuietly(input);
        }
        return contents.toArray(new LoginEvent[contents.size()]);
    }

    public void setAlarmContext(AlarmContext alarmContext) {
        m_alarmContext = alarmContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setLogDirectory(String logDirectory) {
        m_logDirectory = logDirectory;
    }

    /**
     * Log WEB authentication attempts
     */
    public void onApplicationEvent(ApplicationEvent event) {
        if (!(event instanceof InteractiveAuthenticationSuccessEvent)
                && !(event instanceof AbstractAuthenticationFailureEvent)) {
            return;
        }

        AbstractAuthenticationEvent authEvent = (AbstractAuthenticationEvent) event;
        final String username = authEvent.getAuthentication().getName();
        final boolean success = event instanceof InteractiveAuthenticationSuccessEvent;
        if (!success) {
            m_alarmContext.raiseAlarm("LOGIN_FAILED", username);
        }

        Object details = authEvent.getAuthentication().getDetails();
        if (details instanceof WebAuthenticationDetails) {
            String remoteIp = null;
            WebAuthenticationDetails webDetails = (WebAuthenticationDetails) details;
            remoteIp = webDetails.getRemoteAddress();
            logLoginAttempt(username, success, remoteIp);
        }
    }
}
