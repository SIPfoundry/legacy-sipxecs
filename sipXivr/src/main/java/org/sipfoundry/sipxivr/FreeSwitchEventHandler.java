/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

public abstract class FreeSwitchEventHandler {
    protected boolean m_finished = false;
    protected boolean m_hungup = false;

    public boolean isFinished() {
        return m_finished;
    }

    public boolean isHungup() {
        return m_hungup;
    }

    public abstract boolean start() ;

    public abstract boolean handleEvent(FreeSwitchEvent event) ;
}
