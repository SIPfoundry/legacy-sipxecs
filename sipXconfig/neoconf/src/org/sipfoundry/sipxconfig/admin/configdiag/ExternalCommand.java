/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.configdiag;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class ExternalCommand implements Serializable {
    private static final Log LOG = LogFactory.getLog(ExternalCommand.class);
    
    private String m_command;
    private List<String> m_args;
    private ExternalCommandContext m_context;

    public ExternalCommand() {
        m_args = new ArrayList<String>();
    }

    public int execute() {
        String absolutePath = m_command;
        if (!m_command.startsWith("/")) {
            absolutePath = m_context.getBinDirectory() + '/' + m_command;
        }
        StringBuffer commandBuffer = new StringBuffer(absolutePath);
        for (String arg : m_args) {
            Pattern pattern = Pattern.compile("\\$\\{(.*)\\}");
            Matcher matcher = pattern.matcher(arg.trim());
            if (matcher.matches()) {
                String key = matcher.group(1);
                arg = m_context.resolveArgumentString(key);
            }
            commandBuffer.append(' ');
            commandBuffer.append(arg);
        }
        try {
            Process process = Runtime.getRuntime().exec(commandBuffer.toString());
            int exitStatus = process.waitFor();
            LOG.debug("Exit code for command '" + commandBuffer.toString() 
                    + "' was '" + exitStatus + "'");
            return exitStatus;
        } catch (Exception e) {
            return Integer.MIN_VALUE;
        }
    }

    public String getCommand() {
        return m_command;
    }

    public void setCommand(String command) {
        m_command = command;
    }
    
    public void setContext(ExternalCommandContext context) {
        m_context = context;
    }

    public List<String> getArgs() {
        return m_args;
    }

    public void addArgument(String arg) {
        m_args.add(arg);
    }
}
