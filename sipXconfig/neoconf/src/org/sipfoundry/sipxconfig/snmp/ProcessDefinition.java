/**
 *
 *
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
package org.sipfoundry.sipxconfig.snmp;

import static java.lang.String.format;

public final class ProcessDefinition {
    private String m_process;
    private String m_restartCommand;
    private String m_regexp;
    private String m_restartClass;

    private ProcessDefinition(String process) {
        m_process = process;
    }

    private ProcessDefinition(String process, String regexp) {
        this(process);
        m_regexp = regexp;
    }

    public static ProcessDefinition sipx(String process) {
        ProcessDefinition pd = new ProcessDefinition(process);
        pd.setSipxServiceName(process);
        return pd;
    }

    public static ProcessDefinition sipx(String process, String service) {
        ProcessDefinition pd = new ProcessDefinition(process);
        pd.setSipxServiceName(service);
        return pd;
    }

    public static ProcessDefinition sipxByRegex(String process, String regexp) {
        ProcessDefinition pd = new ProcessDefinition(process, regexp);
        pd.setSipxServiceName(process);
        return pd;
    }

    public static ProcessDefinition sipxByRegex(String process, String regexp, String service) {
        ProcessDefinition pd = new ProcessDefinition(process, regexp);
        pd.setSipxServiceName(service);
        return pd;
    }

    public static ProcessDefinition sysv(String process) {
        ProcessDefinition pd = new ProcessDefinition(process);
        pd.setSysVServiceName(process);
        return pd;
    }

    public static ProcessDefinition sysv(String process, String service) {
        ProcessDefinition pd = new ProcessDefinition(process);
        pd.setSysVServiceName(service);
        return pd;
    }

    public static ProcessDefinition sysvByRegex(String process, String regexp) {
        ProcessDefinition pd = new ProcessDefinition(process, regexp);
        pd.setSysVServiceName(process);
        return pd;
    }

    public static ProcessDefinition sysvByRegex(String process, String regexp, String service) {
        ProcessDefinition pd = new ProcessDefinition(process, regexp);
        pd.setSysVServiceName(service);
        return pd;
    }

    public String getProcess() {
        return m_process;
    }

    public String getRegexp() {
        return m_regexp;
    }

    public void setSipxServiceName(String service) {
        setServiceStartCommand(format("$(sipx.SIPX_SERVICEDIR)/%s start", service), service);
    }

    public void setSysVServiceName(String service) {
        setServiceStartCommand(format("/etc/init.d/%s start", service), service);
    }

    private void setServiceStartCommand(String restartCommand, String service) {
        setServiceStartCommand(restartCommand);
        m_restartClass = "restart_" + service;
    }

    public void setServiceStartCommand(String restartCommand) {
        m_restartCommand = restartCommand;
    }

    public String getRestartCommand() {
        return m_restartCommand;
    }

    public String getRestartClass() {
        return m_restartClass;
    }

    public void setRestartClass(String restartClass) {
        m_restartClass = restartClass;
    }
}
