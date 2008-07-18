/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

public class Answer extends CallCommand {

    public Answer(FreeSwitchEventSocketInterface fses) {
        super(fses);
        m_command = "answer";
    }
}
