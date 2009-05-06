/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.voicemail;

import java.io.IOException;
import java.io.OutputStream;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.log4j.Logger;
import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpServer;
import org.mortbay.jetty.servlet.ServletHandler;
import org.sipfoundry.sipxivr.Configuration;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.User;
import org.sipfoundry.sipxivr.ValidUsersXML;

/**
 * Return the status (heard/unheard messages) of a mailbox using a simple HTTP request.
 * 
 * Replaces the existing CGI script used by the Status Server process.
 *
 */
public class Mwistatus extends HttpServlet {
    private static final long serialVersionUID = 2609976094457923038L;
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    
    public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {

        response.setContentType(Mwi.MessageSummaryContentType);
        // Use the OutputStream rather than the PrintWriter as this will cause Jetty
        // To NOT set the charset= parameter on the content type, which breaks
        // The status server doing this request.
        // The price is we aren't specifying the encoding of this message (sigh).
        OutputStream os = response.getOutputStream();
        int heard = 0;
        int unheard = 0;
        int heardUrgent = 0;
        int unheardUrgent = 0;

        // read the query string
        String idUri = request.getParameter("identity");
        
        // Load the list of valid users 
        // (it is static, so don't worry about sucking it in each time, it'll only 
        // be re-read if it has changed on disk)
        ValidUsersXML validUsers = ValidUsersXML.update(true);
        User user = validUsers.isValidUser(ValidUsersXML.getUserPart(idUri));
        if (user != null) {
            // determine the message counts for the mailbox
            // (Okay, worry about this one.  It walks the mailstore directories counting .xml and .sta files.)
            Messages messages = new Messages(new Mailbox(user));
            heard = messages.getHeardCount();
            unheard = messages.getUnheardCount();
            // No support for urgent messages
    
            LOG.info(String.format("Mwistatus::doGet %s %d/%d", idUri, unheard, heard));
        } else {
            // Just lie and give no messages
            LOG.info(String.format("Mwistatus::doGet %s not found", idUri));
        }

        // Just dump the bytes of the string.  No character encoding or nothing.
        os.write(Mwi.formatRFC3842(unheard, heard, unheardUrgent, heardUrgent).getBytes());
        os.close();
    }
    
    
    /**
     * Start the servlet that handles MWI requests
     * @param s_config 
     */
    public static void StartMWIServlet(Configuration s_config, String path) {
        try {
            // TODO change status-plugin.xml to point to https://{machine}:port/mwi
            
            // Start up jetty
            HttpServer server = new HttpServer();

            // Bind the port on all interfaces
            // TODO HTTPS support
            int httpsPort = org.sipfoundry.sipxivr.Configuration.get().getHttpsPort();
            server.addListener(":" + httpsPort);

            HttpContext httpContext = new HttpContext();
            httpContext.setContextPath("/");

            // Setup the servlet to call the Mwistatus class when the URL is fetched
            ServletHandler servletHandler = new ServletHandler();
            servletHandler.addServlet("mwistatus", path, Mwistatus.class.getName());
            httpContext.addHandler(servletHandler);

            server.addContext(httpContext);
            
            // Start it up.
            LOG.info(String.format("Starting MWI servlet on *:%d%s", httpsPort, path));
            server.start();
        } catch (Exception e) {
            e.printStackTrace(); 
        }
    }

}
