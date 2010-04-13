/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.xmlrpc;

import java.util.Hashtable;
import java.util.Vector;

import org.apache.xmlrpc.WebServer;


/**
 * Simple XML RPC server used for test
 */
public class Server {
    private WebServer m_server;

    Server() {
        int port = 9997;
        m_server = new WebServer(port);
        m_server.addHandler("$default", this);
        m_server.start();
    }

    void stop() {
        m_server.shutdown();
    }

    public String multiplyTest(String test, int times) {
        StringBuffer buffer = new StringBuffer();
        for (int i = 0; i < times; i++) {
            buffer.append(test);
        }
        return buffer.toString();
    }

    public int calculateTest(Vector names) {
        int len = 0;
        for (int i = 0; i < names.size(); i++) {
            len += ((String) names.get(i)).length();
        }
        return len;
    }

    public Hashtable create(Hashtable map) {
        return map;
    }
}
