/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import org.apache.log4j.Logger;

public class CallCommand extends FreeSwitchEventHandler {

    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    protected FreeSwitchEventSocketInterface m_fses;
    protected String m_command = "CallCommand";

    public CallCommand(FreeSwitchEventSocketInterface fses) {
        this.m_fses = fses;
    }

    public boolean start() {
        m_finished = false;
        // Send the command to the socket
        m_fses.cmd("sendmsg\ncall-command: execute\nexecute-app-name: " + m_command);
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
