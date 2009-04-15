/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.time;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;
import java.util.TimeZone;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.UserException;

public class TimeManagerImpl implements TimeManager {
    private static final String SIPX_SUDO_TIME = "sipx-sudo-time-manager";
    private static final String SIPX_TIME = "sipx-time-manager";
    private static final String TIMEZONE_BINARY = "sipx-sudo-timezone";
    private static final String SPACE = " ";
    private static final String NEW_LINE = "\n";
    private static final String SCRIPT_ERROR_MESSAGE_KEY = "&message.scriptError";

//    private static final SimpleDateFormat CHANGE_DATE_FORMAT = new SimpleDateFormat(
//            "MMddHHmmyyyy");

    private static final Log LOG = LogFactory.getLog(TimeManagerImpl.class);

    private String m_libExecDirectory;

    private String m_binDirectory;

    private String m_ntpConfigFile;

    public String getLibExecDirectory() {
        return m_libExecDirectory;
    }

    public void setLibExecDirectory(String libExecDirectory) {
        m_libExecDirectory = libExecDirectory;
    }

    public String getBinDirectory() {
        return m_binDirectory;
    }

    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    public String getNtpConfigFile() {
        return m_ntpConfigFile;
    }

    public void setNtpConfigFile(String ntpConfigFile) {
        m_ntpConfigFile = ntpConfigFile;
    }

    public CommandOutput executeCommand(String command, String... parameters) throws IOException,
            InterruptedException {
        // executes command with parameters
        // returns a 3 item String[] which contains at the first position, the return code, the
        // second = the stdout of the command and the third the stderr of the command
        ProcessBuilder pb = new ProcessBuilder(command);
        for (int i = 0; i < parameters.length; i++) {
            pb.command().add(parameters[i]);
        }
        LOG.debug(pb.command());
        Process process = pb.start();

        BufferedReader scriptOutputReader = new BufferedReader(new InputStreamReader(process
                .getInputStream()));
        BufferedReader scriptErrorReader = new BufferedReader(new InputStreamReader(process
                .getErrorStream()));

        String line;
        String errors = new String();
        String output = new String();

        while ((line = scriptOutputReader.readLine()) != null) {
            output = output + NEW_LINE + line;
        }

        while ((line = scriptErrorReader.readLine()) != null) {
            errors = errors + NEW_LINE + line;
        }

        int code = process.waitFor();

        CommandOutput commandOutput = new CommandOutput(code, output, errors);
        return commandOutput;
    }

    public void setSystemDate(String dateString) {
        String errorMsg = "Error when changing date";
        try {
            CommandOutput output = executeCommand(getLibExecDirectory() + File.separator
                    + SIPX_SUDO_TIME, "--date", dateString);
            handleCommandErrors(output);
            int code = output.getReturnCode();
            if (code != 0) {
                throw new UserException(SCRIPT_ERROR_MESSAGE_KEY, code);
            }
        } catch (IOException e) {
            LOG.error(errorMsg, e);
        } catch (InterruptedException e) {
            LOG.error(errorMsg, e);
        }
    }

    public int getSystemTimeSettingType() {
        // return 0 if ntpd is running, 1 if it's not
        try {
            CommandOutput output = executeCommand(getLibExecDirectory() + File.separator
                    + SIPX_SUDO_TIME, "--status");
            handleCommandErrors(output);
            int code = output.getReturnCode();
            return code == 0 ? 0 : 1;
        } catch (IOException e) {
            LOG.error(e);
        } catch (InterruptedException e) {
            LOG.error(e);
        }
        return 1;
    }

    public String getNtpConfiguration() {
        try {
            CommandOutput output = executeCommand(getBinDirectory() + File.separator
                    + SIPX_TIME, "--display-configuration");
            handleCommandErrors(output);
            return output.getStandardOutput();
        } catch (IOException e) {
            LOG.error(e);
        } catch (InterruptedException e) {
            LOG.error(e);
        }
        return "";
    }

    public void setNtpConfiguration(String configuration) {
        try {
            BufferedWriter configFileWriter = new BufferedWriter(new FileWriter(getNtpConfigFile()));
            configFileWriter.write(configuration);
            configFileWriter.flush();
            configFileWriter.close();

            CommandOutput output = executeCommand(getLibExecDirectory() + File.separator
                    + SIPX_SUDO_TIME, "--set-configuration", m_ntpConfigFile);
            handleCommandErrors(output);
            int code = output.getReturnCode();
            if (code != 0) {
                throw new UserException(SCRIPT_ERROR_MESSAGE_KEY, code);
            }
        } catch (IOException e) {
            LOG.error(e);
        } catch (InterruptedException e) {
            LOG.error(e);
        }
    }

    public List<String> getNtpServers() {
        ArrayList<String> al = new ArrayList<String>();
        try {
            CommandOutput output = executeCommand(getBinDirectory() + File.separator
                    + SIPX_TIME, "--get-servers");
            handleCommandErrors(output);
            StringTokenizer st = new StringTokenizer(output.getStandardOutput(), NEW_LINE);
            StringTokenizer st2;
            while (st.hasMoreTokens()) {
                st2 = new StringTokenizer(st.nextToken(), SPACE);
                if (st2.nextToken().equalsIgnoreCase("server")) {
                    al.add(st2.nextToken());
                }
            }
        } catch (IOException e) {
            LOG.error(e);
        } catch (InterruptedException e) {
            LOG.error(e);
        }
        return al;
    }

    public void setNtpServers(List<String> ntpServers) {
        ArrayList<String> al = new ArrayList<String>();
        for (String ntpServer : ntpServers) {
            al.add("--ntp-server");
            al.add(ntpServer);
        }
        try {
            CommandOutput output = executeCommand(getLibExecDirectory() + File.separator
                    + SIPX_SUDO_TIME, al.toArray(new String[0]));
            handleCommandErrors(output);
            int code = output.getReturnCode();
            if (code != 0) {
                throw new UserException(SCRIPT_ERROR_MESSAGE_KEY, code);
            }
        } catch (IOException e) {
            LOG.error(e);
        } catch (InterruptedException e) {
            LOG.error(e);
        }
    }

    public void setSystemTimezone(String timezone) {
        String errorMsg = "Error when changing time zone";
        ProcessBuilder pb = new ProcessBuilder(getLibExecDirectory() + File.separator + TIMEZONE_BINARY);
        pb.command().add(timezone);
        try {
            LOG.debug(pb.command());
            Process process = pb.start();
            BufferedReader scriptErrorReader = new BufferedReader(new InputStreamReader(process.getErrorStream()));
            String errorLine = scriptErrorReader.readLine();
            while (errorLine != null) {
                LOG.warn("sipx-sudo-timezone: " + errorLine);
                errorLine = scriptErrorReader.readLine();
            }
            int code = process.waitFor();
            if (code != 0) {
                errorMsg = String.format("Error when changing time zone. Exit code: %d", code);
                LOG.error(errorMsg);
            }
            TimeZone tz = TimeZone.getTimeZone(timezone);
            TimeZone.setDefault(tz);
        } catch (IOException e) {
            LOG.error(errorMsg, e);
        } catch (InterruptedException e) {
            LOG.error(errorMsg, e);
        }
    }

    private void handleCommandErrors(CommandOutput output) {
        if (!output.getErrorOutput().equals(StringUtils.EMPTY)) {
            // if we have errors, log them
            LOG.warn(output.getErrorOutput());
        }
    }

    private class CommandOutput {
        private final int m_returnCode;
        private final String m_standardOutput;
        private final String m_errorOutput;

        public CommandOutput(int returnCode, String standardOutput,
                String errorOutput) {
            m_returnCode = returnCode;
            m_standardOutput = standardOutput;
            m_errorOutput = errorOutput;
        }

        public int getReturnCode() {
            return m_returnCode;
        }

        public String getStandardOutput() {
            return m_standardOutput;
        }

        public String getErrorOutput() {
            return m_errorOutput;
        }
    }
}
