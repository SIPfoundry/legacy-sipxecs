/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.freeswitch;

import java.io.IOException;
import java.net.Socket;
import java.util.Vector;

import org.sipfoundry.commons.freeswitch.FreeSwitchEvent;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;

public class FreeSwitchEventSocketEmulator extends FreeSwitchEventSocketInterface {

    public FreeSwitchEventSocketEmulator(FreeSwitchConfigurationInterface config) {
        super(config);
    }

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
    public boolean connect(Socket socket) throws IOException {
        return true;
    }

    @Override
    public boolean connect(Socket socket, String authPassword) throws IOException {
    	return true;
    }

}
