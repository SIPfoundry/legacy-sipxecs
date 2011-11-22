/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.conference;

import java.io.IOException;
import java.io.PrintWriter;
import java.net.Socket;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.FreeSwitchConfigurationInterface;
import org.sipfoundry.commons.freeswitch.FreeSwitchEvent;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocket;
import org.sipfoundry.commons.timeout.Result;
import org.sipfoundry.commons.timeout.SipxExecutor;
import org.sipfoundry.commons.timeout.Timeout;
import org.sipfoundry.sipxivr.rest.SipxIvrServletHandler;
import org.sipfoundry.voicemail.IvrLocalizer;

public class ConferenceServlet extends HttpServlet {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    @Override
    public void doPut(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        PrintWriter pw = response.getWriter();
        String pathInfo = request.getPathInfo();
        String commandStr = "conference" + pathInfo.replace('/', ' ');

        response.setContentType("text/xml");

        FreeSwitchConfigurationInterface fsConfig = (FreeSwitchConfigurationInterface) request
                .getAttribute(SipxIvrServletHandler.FS_CONFIG_ATTR);
        executeCommand(commandStr.trim(), new IvrLocalizer(), pw, fsConfig);

        pw.close();
    }

    private synchronized void executeCommand(String cmd, IvrLocalizer localizer, PrintWriter pw,
            FreeSwitchConfigurationInterface fsConfig) {

        FreeSwitchEventSocket fsEventSocket = new FreeSwitchEventSocket(fsConfig);
        Socket socket = null;
        try {
            socket = new Socket("localhost", 8021);
            if (fsEventSocket.connect(socket, "ClueCon")) {
                Result result = null;
                try {
                    result = SipxExecutor.execute(new TimeoutCommand(fsEventSocket, cmd), 60);
                    pw.format("<command-response>%s</command-response>\n", result.getResult());
                } catch (Exception ex) {
                    pw.format("<command-exception>%s</command-exception>\n", ex.getMessage());
                }
            }
        } catch (IOException ioEx) {
            LOG.error("failed to executeCommand" + ioEx.getMessage());
        } finally {
            try {
                fsEventSocket.close();
            } catch (IOException ex) {
                LOG.error("failed to close FS socket" + ex.getMessage());
            }

        }

    }

    private class TimeoutCommand implements Timeout {
        FreeSwitchEventSocket m_socket;
        String m_command;

        public TimeoutCommand(FreeSwitchEventSocket socket, String command) {
            m_socket = socket;
            m_command = command;
        }

        @Override
        public Result timeoutMethod() {
            FreeSwitchEvent event = m_socket.apiCmdResponse(m_command);
            return new Result(true, event.getContent());
        }
    }

}
