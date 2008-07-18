/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

public class Hangup extends CallCommand {

    public Hangup(FreeSwitchEventSocketInterface fses) {
        super(fses);
        m_command = "hangup";
    }
}
