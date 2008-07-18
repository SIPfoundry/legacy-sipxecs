/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import java.io.IOException;
import java.net.Socket;
import java.util.Vector;

import org.sipfoundry.sipxivr.FreeSwitchEvent;
import org.sipfoundry.sipxivr.FreeSwitchEventSocketInterface;

public class FreeSwitchEventSocketEmulator extends FreeSwitchEventSocketInterface {

    public Vector<String> cmds = new Vector<String>();
    public FreeSwitchEvent event;

    @Override
    public FreeSwitchEvent awaitLiveEvent() {
        return event;
    }

    @Override
    public void close() throws IOException {
    }

    @Override
    public void cmd(String cmd) {
        cmds.add(cmd);
    }

    @Override
    public FreeSwitchEvent cmdResponse(String cmd) {
        cmds.add(cmd);
        return event;
    }

    @Override
    public void connect(Socket socket) throws IOException {
    }

}
