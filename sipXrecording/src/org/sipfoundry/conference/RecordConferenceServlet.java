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

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.log4j.Logger;

public class RecordConferenceServlet extends ConferenceServlet {
    private static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxrecording");

    private static String IN_PROGRESS = "IN_PROGRESS";
    private static String NO_RECORDING = "NO_RECORDING";

    @Override
    public void doPut(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        PrintWriter pw = response.getWriter();
        String parmAction = request.getParameter("action");
        String parmConf = request.getParameter("conf");

        boolean stringsOK = parmAction != null && !parmAction.isEmpty() &&
                            parmConf != null && !parmConf.isEmpty();

        if (stringsOK) {
            String wavPath = ConferenceContextImpl.getInstance().getWavPath(parmConf);
            if (parmAction.equals("start")) {
                if (!ConferenceContextImpl.getInstance().isRecordingInProgress(parmConf)) {
                    ConferenceContextImpl.getInstance().createSourceDir();
                    executeCommand("conference "+parmConf+" record " + wavPath, pw);
                } else {
                    pw.format("<command-response>%s</command-response>\n", IN_PROGRESS);
                }
            } else if (parmAction.equals("stop")) {
                executeCommand("conference "+parmConf+" norecord " + wavPath, pw);
                ConferenceContextImpl.getInstance().saveInMailboxSynch(parmConf);
            } else if (parmAction.equals("status")) {
                if (ConferenceContextImpl.getInstance().isRecordingInProgress(parmConf)) {
                    pw.format("<command-response>%s</command-response>\n", IN_PROGRESS);
                } else {
                    pw.format("<command-response>%s</command-response>\n", NO_RECORDING);
                }
            } else if (parmAction.equals("duration")) {
                pw.format("<command-response>%s</command-response>\n", getRecordingDuration(wavPath));
            } else {
                pw.format("<command-response>%s</command-response>\n", "ERROR: Incorect \"action\" parameter");
            }
        } else {
            pw.format("<command-response>%s</command-response>\n", "ERROR: Wrong parameters");
        }

        response.setContentType("text/xml");
        pw.close();
    }

    private boolean isRecordingInProgress(File wav) {
        return wav.exists();
    }

    private long getRecordingDuration(String wavPath) {
        File wav = new File(wavPath);
        return isRecordingInProgress(wav) ? wav.length()/(8000*2) : 0;
    }
}
