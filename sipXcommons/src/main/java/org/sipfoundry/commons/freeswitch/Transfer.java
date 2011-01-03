/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.freeswitch;

public class Transfer extends CallCommand {

    private String m_uuid;

    public Transfer(FreeSwitchEventSocketInterface fses, String sipURI) {
        super(fses);
        // Send a REFER
        createCommand(sipURI);
    }

    public Transfer(FreeSwitchEventSocketInterface fses, String uuid, String sipURI) {
        super(fses);
        m_uuid = uuid;
        createCommand(sipURI);
    }

    private void createCommand(String sipURI) {
        m_command = "deflect\nexecute-app-arg: ";
        // sipURI MUST have sip: in there (Can be display URI)
        if (sipURI.toLowerCase().contains("sip:")) {
            m_command += sipURI;
        } else {
            m_command += "sip:" + sipURI;
        }
    }

    @Override
    public void go() {
        if(m_uuid == null) {
            super.go();
        } else {
            m_finished = false;
            // Send the command to the socket
            m_fses.cmd("sendmsg " + m_uuid +
                    "\ncall-command: execute\nexecute-app-name: " + m_command);
        }
    }
}
