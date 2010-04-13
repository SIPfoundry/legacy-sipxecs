/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site;

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;

import junit.extensions.TestSetup;
import junit.framework.Test;
import junit.framework.TestCase;
import org.mortbay.jetty.Server;
import org.mortbay.jetty.servlet.JettyWebConfiguration;
import org.mortbay.jetty.servlet.WebApplicationContext;
import org.mortbay.jetty.servlet.XMLConfiguration;
import org.mortbay.util.InetAddrPort;

public class JettyTestSetup extends TestSetup {

    private static boolean START_SERVER = true;

    private static final int MONITOR_PORT = 9998;

    private static final String MONITOR_KEY = "sipxconfig";

    private static Server m_server;

    private final int m_port = 9999;

    private final String m_url = "http://localhost:" + m_port + "/sipxconfig";

    static {
        // shows which URLs were accessed among other diagnotics
        // but slows down testing.
        // System.setProperty("DEBUG", "true");
    }

    public JettyTestSetup(Test test) {
        super(test);
    }

    public String getUrl() {
        return m_url;
    }

    /**
     * "Leaks" the web server on purpose, but does gracefully shutdown server when JVM shutsdown.
     * First test will start server, subsequent tests will use shared server instance.
     */
    @Override
    protected void setUp() throws Exception {
        if (START_SERVER && m_server == null) {
            m_server = startServer();
        }
    }

    protected Server startServer() throws Exception {
        // uncomment to disable page and component caching
        // System.setProperty("org.apache.tapestry.disable-caching", "true");
        Server server = new Server();
        server.addListener(new InetAddrPort(m_port));
        // add the sipXconfig web application
        String war = SiteTestHelper.getBuildDirectory() + "/tests/war";
        WebApplicationContext context = server.addWebApplication("/sipxconfig", war);

        String[] configurationClassNames = {
            XMLConfiguration.class.getName(), JettyTestWebConfiguration.class.getName()
        };
        context.setConfigurationClassNames(configurationClassNames);

        // start monitor thread that allows stopping server
        Monitor.monitor(MONITOR_PORT, MONITOR_KEY);

        server.start();
        return server;
    }

    private static void shutdownJetty() throws UnknownHostException, IOException {
        Socket s = new Socket(InetAddress.getByName("127.0.0.1"), MONITOR_PORT);
        OutputStream out = s.getOutputStream();
        OutputStreamWriter writer = new OutputStreamWriter(out);
        writer.write(MONITOR_KEY);
        writer.write("\nstop\n");
        writer.flush();
        s.shutdownOutput();
        s.close();
    }

    private static void startJetty() {
        TestCase notest = new TestCase() {
            // empty
        };
        JettyTestSetup jetty = new JettyTestSetup(notest);
        try {
            m_server = jetty.startServer();
            Runtime.getRuntime().addShutdownHook(new Thread(jetty.shutdownHoook()));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * If you want to run sipXconfig in jetty w/o any tests
     */
    public static void main(String[] args) throws Exception {
        if (args.length > 0 && "shutdown".equals(args[0])) {
            shutdownJetty();
        } else {
            startJetty();
        }
    }

    Runnable shutdownHoook() {
        return new Runnable() {
            public void run() {
                try {
                    m_server.stop();
                    m_server = null;
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        };
    }

    /**
     * Special version of JettyConfiguration that appends class path
     */
    public static class JettyTestWebConfiguration extends JettyWebConfiguration {
        @Override
        public void configureClassPath() throws Exception {
            getWebApplicationContext().addClassPath(SiteTestHelper.getArtificialSystemRootDirectory() + "/etc");
        }

    }
}
