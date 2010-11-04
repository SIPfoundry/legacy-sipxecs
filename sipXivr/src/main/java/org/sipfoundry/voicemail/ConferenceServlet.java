package org.sipfoundry.voicemail;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.Collection;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.ConfBasicThread;
import org.sipfoundry.commons.freeswitch.ConfCommand;
import org.sipfoundry.commons.timeout.Result;
import org.sipfoundry.commons.timeout.SipxExecutor;
import org.sipfoundry.commons.timeout.Timeout;

public class ConferenceServlet extends HttpServlet {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        PrintWriter pw = response.getWriter();
        String [] subDirs = request.getPathInfo().split("/");

        response.setContentType("text/xml");

        executeCommand(getConferenceName(subDirs), getCommand(subDirs), new IvrLocalizer(), pw);

        pw.close();
    }

    private String getCommand(String[] subDirs) {
        if (subDirs.length == 2) {
            return subDirs[1];
        } else if (subDirs.length == 3) {
            return subDirs[2];
        } else {
            return "";
        }
    }

    private String getConferenceName(String[] subDirs) {
        if (subDirs.length == 2) {
            return "";
        } else if (subDirs.length == 3) {
            return subDirs[1];
        } else {
            return "";
        }
    }

    private synchronized void executeCommand(String confName, String cmd, IvrLocalizer localizer, PrintWriter pw) {


        ConfCommand confcmd = new ConfCommand(ConfBasicThread.getCmdSocket(), confName, cmd, localizer);
        Result result = null;
        try {
            result = SipxExecutor.execute(new TimeoutCommand(confcmd), 60);
            pw.format("<command-executed>%s</command-executed>\n", result.isSuccesfull());
            pw.format("<command-response>%s</command-response>\n", confcmd.getResponse());
        } catch(Exception ex) {
            pw.format("<command-exception>%s</command-exception>\n", ex.getClass().getName()+" "+ex.getMessage());
        }
        if (confcmd.isSucessful()) {
            LOG.debug("Conf command SUCCESS " + cmd + " " + confcmd.getResponse());
            pw.format("<command-response>%s</command-response>\n", confcmd.getResponse());
        } else {
            LOG.debug("Conf command FAILED " + cmd + " " + confcmd.GetErrString());
            pw.format("<command-error>%s</command-error>\n", confcmd.GetErrString());
        }
    }

    private class TimeoutCommand implements Timeout {
        ConfCommand m_command;

        public TimeoutCommand(ConfCommand command) {
            m_command = command;
        }
        @Override
        public Result timeoutMethod() {
            m_command.go();
            return new Result(true, null);
        }

    }

}
