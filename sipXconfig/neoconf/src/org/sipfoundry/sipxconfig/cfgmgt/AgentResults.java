/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
