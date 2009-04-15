/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

public class Set extends CallCommand {

    public Set(FreeSwitchEventSocketInterface fses, String variable, String value) {
        super(fses);
        m_command = String.format("set\nexecute-app-arg: %s=%s", variable, value);
    }
}
