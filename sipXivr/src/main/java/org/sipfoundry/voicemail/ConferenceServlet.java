/*
 * Copyright (c) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.voicemail;

import java.io.IOException;
import java.io.PrintWriter;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.sipfoundry.commons.freeswitch.ConfBasicThread;
import org.sipfoundry.commons.freeswitch.FreeSwitchEvent;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocket;
import org.sipfoundry.commons.timeout.Result;
import org.sipfoundry.commons.timeout.SipxExecutor;
import org.sipfoundry.commons.timeout.Timeout;

public class ConferenceServlet extends HttpServlet {

    public void doPut(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        PrintWriter pw = response.getWriter();
        String pathInfo = request.getPathInfo();
        String commandStr = "conference"+pathInfo.replace('/', ' ');

        response.setContentType("text/xml");

        executeCommand(commandStr.trim(), new IvrLocalizer(), pw);

        pw.close();
    }

    private synchronized void executeCommand(String cmd, IvrLocalizer localizer, PrintWriter pw) {

        FreeSwitchEventSocket socket = ConfBasicThread.getCmdSocket();
        Result result = null;
        try {
            result = SipxExecutor.execute(new TimeoutCommand(socket, cmd), 60);
            pw.format("<command-response>%s</command-response>\n", result.getResult());
        } catch(Exception ex) {
            pw.format("<command-exception>%s</command-exception>\n", ex.getMessage());
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
