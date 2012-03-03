/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Stack;

public class AgentResults {
    private Stack<String> m_errors;
    private Thread m_runner;

    public void parse(final InputStream in) {
        m_runner = new Thread("sipxagent error reader") {
            @Override
            public void run() {
                parseInput(in);
            }
        };
        m_runner.start();
    }

    void parseInput(InputStream in) {
        m_errors = new Stack<String>();
        try {
            BufferedReader r = new BufferedReader(new InputStreamReader(in));
            while (true) {
                String line = r.readLine();
                if (line == null) {
                    break;
                }
                m_errors.push(line);
            }
        } catch (IOException e) {
            m_errors.push("Could not read agent results. " + e.getMessage());
        }
    }

    public Stack<String> getResults(long timeout) throws InterruptedException {
        m_runner.join(timeout);
        return m_errors;
    }
}
