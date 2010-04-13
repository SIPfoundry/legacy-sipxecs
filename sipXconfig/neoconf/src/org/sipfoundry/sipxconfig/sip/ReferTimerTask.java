/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.sip;

import java.util.TimerTask;

import javax.sip.Dialog;
import javax.sip.DialogState;

public class ReferTimerTask extends TimerTask {
    private Dialog m_dialog;

    public ReferTimerTask(Dialog dialog) {
        m_dialog = dialog;
    }

    @Override
    public void run() {
        if (m_dialog.getState() != DialogState.TERMINATED) {
            m_dialog.delete();
        }
    }

}
