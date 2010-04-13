/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.freeswitch;

public class Set extends CallCommand {

    String m_uuid;

    public boolean start() {
        if(m_uuid == null) {
            return super.start();
        }
        m_finished = false;
        // Send the command to the socket
        m_fses.cmd("sendmsg " + m_uuid +
                "\ncall-command: execute\nexecute-app-name: " + m_command);
        return false;
    }


    public boolean handleEvent(FreeSwitchEvent event) {

        if(m_uuid == null) {
            return super.handleEvent(event);
        }

        return true;
    }
   
    public Set(FreeSwitchEventSocketInterface fses, String variable, String value) {
        super(fses);
        m_uuid = null;
        m_command = String.format("set\nexecute-app-arg: %s=%s", variable, value);
    }

    public Set(FreeSwitchEventSocketInterface fses, String chan_uuid, String variable, String value) {
        super(fses);
        m_uuid = chan_uuid;
        m_command = "event\nexecute-app-arg: " + variable + "=" + value;
    }      
}
