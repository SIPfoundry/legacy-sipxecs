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

/*
 * emove the mechanism that 
components rely on without proposing an upgrade strategy. I do not want to 
shut up any discussions on any type of changes in configuration protocols 
but I just do not see any type of revolution coming. (which might just mean 
I am not looking close enough).

 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement. Licensed to
 * the User under the LGPL license. $
 */
public class ExternalCommand implements Serializable {
    private String m_command;
    private List<String> m_args;

    public ExternalCommand() {
        m_args = new ArrayList<String>();
    }

    public int execute() {
        StringBuffer commandBuffer = new StringBuffer(m_command);
        for (String arg : m_args) {
            commandBuffer.append(' ');
            commandBuffer.append(arg);
        }
        try {
            Process process = Runtime.getRuntime().exec(commandBuffer.toString());
            int exitStatus = process.waitFor();
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

    public List<String> getArgs() {
        return m_args;
    }

    public void addArgument(String arg) {
        m_args.add(arg);
    }
}
