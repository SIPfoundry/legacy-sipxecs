/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.freeswitch;

import org.apache.log4j.Logger;

public class CallCommand extends FreeSwitchEventHandler {

    protected final Logger LOG;
    protected FreeSwitchEventSocketInterface m_fses;
    protected String m_command = "CallCommand";
    protected boolean m_sendAsApi = false;

    public CallCommand(FreeSwitchEventSocketInterface fses) {
        this.m_fses = fses;
        this.LOG = fses.getConfig().getLogger();
    }

    public boolean start() {
        m_finished = false;
        // Send the command to the socket
        if (!m_sendAsApi)
            m_fses.cmd("sendmsg " + m_fses.getSessionUUID() + "\ncall-command: execute\nexecute-app-name: " + m_command);
        else
            m_fses.cmd("api " + m_command);
        return false;
    }

    public void go() {
        m_fses.invoke(this);
    }

    public boolean handleEvent(FreeSwitchEvent event) {

        if (event.isEmpty()) {
            m_hungup = true;
        }

        if (event.isEmpty()
                || event.getEventValue("Event-Name", "")
                        .contentEquals("CHANNEL_EXECUTE_COMPLETE")) {
            // note command is finished.
            m_finished = true;
        }
        return isFinished();
    }

}
