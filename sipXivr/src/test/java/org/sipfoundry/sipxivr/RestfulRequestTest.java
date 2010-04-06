/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxivr;

import java.util.Properties;

import junit.framework.TestCase;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpServer;
import org.mortbay.jetty.servlet.ServletHandler;

public class RestfulRequestTest extends TestCase {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    HttpServer m_server;
    boolean dontBother = true;
    
    protected void setUp() throws Exception {
        super.setUp();
        // Configure log4j
        Properties props = new Properties();
        props.setProperty("log4j.rootLogger", "debug, cons");
        props.setProperty("log4j.appender.cons", "org.apache.log4j.ConsoleAppender");
        props.setProperty("log4j.appender.cons.layout", "org.sipfoundry.commons.log4j.SipFoundryLayout");
        props.setProperty("log4j.appender.cons.layout.facility", "sipXivr");

        PropertyConfigurator.configure(props);

     // Start up jetty
        try {
            m_server = new HttpServer();
            m_server.addListener("localhost:" + 12345);
            HttpContext httpContext = new HttpContext();
            httpContext.setContextPath("/");
            ServletHandler servletHandler = new ServletHandler();
            servletHandler.addServlet("RestfulRequestTest", "/woof/*", RestfulRequestTestServlet.class.getName());
            httpContext.addHandler(servletHandler);
            m_server.addContext(httpContext);
            m_server.start();
        } catch (Exception e) {
            dontBother = true ;
            LOG.warn("Problem starting Jetty.  Skip tests.", e);
        }

    }

    protected void tearDown() throws Exception {
        super.tearDown();
        m_server.stop(true);
    }

    public void testPut() {
        if (dontBother)
            return ;
        
        RestfulRequest rr = new RestfulRequest("http://localhost:12345/woof/dog");
        try {
            boolean okay = rr.put("put");
            assertTrue("response not okay", okay);
        } catch (Exception e) {
            fail("Exception "+e);
        }
    }

    public void testPost() {
        if (dontBother)
            return ;

        RestfulRequest rr = new RestfulRequest("http://localhost:12345/woof/dog");
        try {
            boolean okay = rr.post("post");
            assertTrue("response not okay", okay);
            assertEquals("Next time, use mail, not POST!\n", rr.getContent());
        } catch (Exception e) {
            fail("Exception "+e);
        }
    }

    public void testDelete() {
        if (dontBother)
            return ;

        RestfulRequest rr = new RestfulRequest("http://localhost:12345/woof/dog");
        try {
            boolean okay = rr.delete();
            assertTrue("response not okay", okay);
        } catch (Exception e) {
            fail("Exception "+e);
        }
    }

    public void testGet() {
        if (dontBother)
            return ;

        RestfulRequest rr = new RestfulRequest("http://localhost:12345/woof/dog");
        try {
            boolean okay = rr.get();
            assertTrue("response not okay", okay);
            assertEquals("How now, fuzzy brown puppy!\n", rr.getContent());
        } catch (Exception e) {
            fail("Exception "+e);
        }
    }

}
