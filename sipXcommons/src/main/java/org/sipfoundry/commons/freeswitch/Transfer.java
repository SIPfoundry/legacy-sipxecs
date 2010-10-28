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

    public Transfer(FreeSwitchEventSocketInterface fses, String sipURI) {
        super(fses);
        // Send a REFER
        m_command = "deflect\nexecute-app-arg: ";
        // sipURI MUST have sip: in there (Can be display URI)
        if (sipURI.toLowerCase().contains("sip:")) {
            m_command += sipURI;
        } else {
            m_command += "sip:" + sipURI;
        }
    }

    public Transfer(FreeSwitchEventSocketInterface fses, String uuid, String sipURI) {
        super(fses);
        m_sendAsApi = true;
        // Send a REFER using a particular uuid
        m_command = "uuid_deflect " + uuid + " " + sipURI;
    }
}
