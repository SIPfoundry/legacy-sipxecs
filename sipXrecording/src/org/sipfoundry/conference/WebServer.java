package org.sipfoundry.conference;

import org.apache.log4j.Logger;
import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpServer;
import org.mortbay.jetty.servlet.ServletHandler;
import org.sipfoundry.sipxrecording.RecordingConfiguration;

public class WebServer {
    private static int PORT = RecordingConfiguration.get().getJettyPort();
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxrecording");
    private static WebServer instance;
    private HttpServer server;
    private WebServer() {
        try {
            server = new HttpServer();
            HttpContext httpContext = new HttpContext();
            httpContext.setContextPath("/");
            ServletHandler handler = new ServletHandler();
            addServlet(handler, "conference", "/conference/*", ConferenceServlet.class.getName());
            addServlet(handler, "recordconference", "/recordconference/*", RecordConferenceServlet.class.getName());
            httpContext.addHandler(0, handler);
            server.addContext(httpContext);
            server.addListener(":" + PORT);
            LOG.info(String.format("Starting Jetty server on port: %d", PORT));
        } catch (Exception e) {
            server = null;
            LOG.error(String.format("Cannot instantiate jetty server on port *:%d", PORT), e);
        }
    }
    public static synchronized WebServer getInstance() {
        if (instance == null) {
            instance = new WebServer();
        }
        return instance;
    }

    /**
     * add a servlet for the Web server to use
     * @param name
     * @param pathSpec
     * @param servletClass must be of type javax.servlet.Servlet
     */
    private void addServlet(ServletHandler handler, String name, String pathSpec, String servletClass) {
        handler.addServlet(name, pathSpec, servletClass);
        LOG.info(String.format("Adding Servlet %s on %s", name, pathSpec));
    }

    public boolean startServer() {
        if (server != null && !server.isStarted()) {
            try {
                server.start();
                return true;
            } catch (Exception ex) {
                LOG.error("Error starting server ", ex);
                return false;
            }
        } else if (server.isStarted()) {
            return true;
        } else {
            LOG.error("Error creating server");
            return false;
        }
    }
}
