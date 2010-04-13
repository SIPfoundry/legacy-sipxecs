/*
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrecording;

import org.sipfoundry.commons.freeswitch.CallCommand;
import org.sipfoundry.commons.freeswitch.FreeSwitchEvent;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;

public class RecordCommand extends CallCommand {
    String  m_command;
    String  m_confName;
    
    public boolean start() {
        m_finished = false;
        // Send the command to the socket
        m_fses.cmd("api conference " + m_confName + " " + m_command);
        return false;
    }

    public RecordCommand(FreeSwitchEventSocketInterface fses, String confName, String confCommand) {
        super(fses);
        m_confName = confName;
        m_command = confCommand;
    }
    
    public boolean handleEvent(FreeSwitchEvent event) {
        // Don't really care what the response is, we're done.
        return true;
    }
}
