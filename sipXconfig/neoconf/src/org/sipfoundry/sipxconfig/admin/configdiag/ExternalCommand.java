/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.configdiag;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class ExternalCommand {
    private static final Log LOG = LogFactory.getLog(ExternalCommand.class);

    private static final Pattern PARAM_PATTERN = Pattern.compile("\\$\\{(.*)\\}");

    private String m_command;
    private final List<String> m_args = new ArrayList<String>();
    private ExternalCommandContext m_context;
    private String m_stdout;

    public int execute() {
        try {
            return rawExecute();
        } catch (IOException e) {
            LOG.error("Cannot execute: " + m_command, e);
            return Integer.MIN_VALUE;
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }

    private int rawExecute() throws IOException, InterruptedException {
        List<String> cmdAndArgs = prepareCommandLine();
        String[] cmdArray = cmdAndArgs.toArray(new String[cmdAndArgs.size()]);
        String fullCommand = StringUtils.join(cmdArray, " ");
        Process process = Runtime.getRuntime().exec(cmdArray);

        InputStream stream = process.getInputStream();

        Reader streamReader = new InputStreamReader(stream);

        StringWriter writer = new StringWriter();
        IOUtils.copy(streamReader, writer);
        int exitStatus = process.waitFor();
        m_stdout = writer.toString();

        LOG.debug(String
                .format("Starting external command output for command '%s':", fullCommand));
        LOG.debug(m_stdout);
        LOG.debug(String.format("Exit code for command '%s' was '%d'", fullCommand, exitStatus));

        return exitStatus;
    }

    private List<String> prepareCommandLine() {
        List<String> cmdAndArgs = new ArrayList<String>();
        cmdAndArgs.add(getFullCommand());
        for (String arg : m_args) {
            Matcher matcher = PARAM_PATTERN.matcher(arg.trim());
            if (matcher.matches()) {
                String key = matcher.group(1);
                arg = m_context.resolveArgumentString(key);
            }
            cmdAndArgs.add(arg);
        }
        return cmdAndArgs;
    }

    private String getFullCommand() {
        String absolutePath = m_command;
        if (!m_command.startsWith("/")) {
            absolutePath = m_context.getBinDirectory() + '/' + m_command;
        }
        return absolutePath;
    }

    public String getCommand() {
        return m_command;
    }

    public void setCommand(String command) {
        m_command = command;
    }

    public void setContext(ExternalCommandContext context) {
        m_context = context;
    }

    public void addArgument(String arg) {
        m_args.add(arg);
    }

    public void addArgument(String arg, int position) {
        m_args.add(position, arg);
    }

    public void removeArgument(int index) {
        m_args.remove(index);
    }

    public void clearArguments() {
        m_args.clear();
    }

    List<String> getArgs() {
        return m_args;
    }

    public String getStdout() {
        return m_stdout;
    }
}
