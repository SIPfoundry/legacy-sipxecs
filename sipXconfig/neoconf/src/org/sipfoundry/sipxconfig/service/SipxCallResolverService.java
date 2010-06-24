/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.io.File;
import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.annotation.Required;

public class SipxCallResolverService extends SipxService implements LoggingEntity {

    public static final String BEAN_ID = "sipxCallResolverService";

    public static final String CDR_REMOTE_ACCESS_IP = "callresolver/SIP_CALLRESOLVER_REMOTE_ACCESS";
    public static final String LOG_SETTING = "callresolver/SIP_CALLRESOLVER_LOG_LEVEL";

    private static  final Log  LOG = LogFactory.getLog(SipxCallResolverService.class);
    private static final String CDRPOSTGRES_BINARY = "sipxcdrpostgres";
    private static final String SPACE = " ";
    private static final String EMPTY_STRING = "";
    private static final int INVALID_PRIVILEGES = 1;
    private static final int INVALID_COMMAND_FORMAT = 2;
    private static final int INVALID_IP_ADDRESS = 3;
    private static final String ERROR = "Errors when executing CDR Postgres script: %s";


    private int m_agentPort;
    private String m_libExecDir;
    private LocationsManager m_locationManager;

    public int getAgentPort() {
        return m_agentPort;
    }

    public void setAgentPort(int agentPort) {
        m_agentPort = agentPort;
    }

    public String getAgentAddress() {
        Location agentLocation = m_locationManager.getPrimaryLocation();
        return agentLocation.getAddress();
    }

    @Required
    public void setLibExecDir(String libExecDir) {
        m_libExecDir = libExecDir;
    }

    public String getLibExecDir() {
        return m_libExecDir;
    }

    @Required
    public void setLocationManager(LocationsManager locationManager) {
        m_locationManager = locationManager;
    }

    protected void execute(String remoteIp) {
        String cmdLine = getCmdLine(remoteIp);
        try {
            Process process = Runtime.getRuntime().exec(cmdLine);
            int code = process.waitFor();
            if (code == INVALID_PRIVILEGES) {
                throw new UserException("&message.invalidPrivileges");
            } else if (code == INVALID_COMMAND_FORMAT) {
                throw new UserException("&message.invalidCommandFormat");
            } else if (code == INVALID_IP_ADDRESS) {
                throw new UserException("&message.invalidIp");
            }
        } catch (IOException e) {
            throw new UserException("&message.noScriptFound");
        } catch (InterruptedException e) {
            LOG.warn(String.format(ERROR, cmdLine + SPACE));
        }
    }

    String getCmdLine(String remoteIp) {
        File executable = new File(getLibExecDir(), CDRPOSTGRES_BINARY);
        String action = "--add";
        String ipaddr = remoteIp;
        if ((remoteIp == null) || remoteIp.equals(EMPTY_STRING)) {
            // Value is default so that means to turn off remote access.
            action = "--delete";
            ipaddr = EMPTY_STRING;
        }
        String cmd = executable.getAbsolutePath() + SPACE + action + SPACE + ipaddr;
        LOG.info("Command to execute is " + cmd);
        return cmd;
    }

    @Override
    public String getLogSetting() {
        return LOG_SETTING;
    }

    @Override
    public void setLogLevel(String logLevel) {
        super.setLogLevel(logLevel);
    }

    @Override
    public String getLogLevel() {
        return super.getLogLevel();
    }

    @Override
    public String getLabelKey() {
        return super.getLabelKey();
    }

    @Override
    public void onConfigChange() {
        String remoteIp = getSettingValue(CDR_REMOTE_ACCESS_IP);
        execute(remoteIp);
    }
}
