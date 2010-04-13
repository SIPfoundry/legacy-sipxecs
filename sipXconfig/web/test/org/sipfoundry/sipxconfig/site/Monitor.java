/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

import java.io.InputStreamReader;
import java.io.LineNumberReader;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * This is copied from examples in Jetty installation
 * (./jetty-5.1.4/src/org/mortbay/start/Monitor.java) and simplified to only support stop command.
 * The original code was licensed under the Apache License, Version 2.0 (the "License")
 *
 * Monitor thread. This thread listens on the port specified by the STOP.PORT system parameter
 * (defaults to 9998) for request authenticated with the key given by the STOP.KEY system
 * parameter (defaults to "sipxconfig") for admin requests.
 */
public class Monitor extends Thread {
    private int m_port;
    private String m_key;

    private ServerSocket m_socket;

    Monitor(int port, String key) {
        m_port = port;
        m_key = key;
        try {
            setDaemon(true);
            m_socket = new ServerSocket(m_port, 1, InetAddress.getByName("127.0.0.1"));
        } catch (Exception e) {
            System.err.println(e.toString());
        }
        if (m_socket != null)
            start();
        else
            System.err.println("WARN: Not listening on monitor port: " + m_port);
    }

    public void run() {
        while (true) {
            Socket socket = null;
            try {
                socket = m_socket.accept();

                LineNumberReader lin = new LineNumberReader(new InputStreamReader(socket
                        .getInputStream()));
                String key = lin.readLine();
                if (!m_key.equals(key))
                    continue;

                String cmd = lin.readLine();
                if ("stop".equals(cmd)) {
                    try {
                        socket.close();
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    try {
                        m_socket.close();
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    System.exit(0);
                }
            } catch (Exception e) {
                System.err.println(e.toString());
            } finally {
                if (socket != null) {
                    try {
                        socket.close();
                    } catch (Exception e) {
                    }
                }
                socket = null;
            }
        }
    }

    /**
     * Start a Monitor. This static method starts a monitor that listens for admin requests.
     */
    public static void monitor(int port, String key) {
        new Monitor(port, key);
    }
}
