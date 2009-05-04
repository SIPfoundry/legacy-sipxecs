/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.voicemail;

import java.io.*;
import java.net.URL;

import javax.servlet.*;
import javax.servlet.http.*;

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
    /**
     * 
     */
    private static final long serialVersionUID = 2609976094457923038L;
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    
    public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {

        response.setContentType("application/simple-message-summary");
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
        os.write(formatRFC3842(unheard, heard, unheardUrgent, heardUrgent).getBytes());
        os.close();
    }
    
    /**
     * Format the status ala RFC-3842
     * 
     * @param numNew
     * @param numOld
     * @param numNewUrgent
     * @param numOldUrgent
     * @return
     */
    String formatRFC3842(int numNew, int numOld, int numNewUrgent, int numOldUrgent) {
        return String.format("Messages-Waiting: %s\r\nVoice-Message: %d/%d (%d/%d)\r\n\r\n",
        		numNew > 0 ? "yes":"no", numNew, numOld, numNewUrgent, numOldUrgent);
    }
    
    /**
     * Start the servlet that handles MWI requests
     * @param s_config 
     */
    public static void StartMWIServlet(Configuration s_config) {
        try {
            
            String mwiStatusURL = null;
            URL url; 

/*            
 * Parse the status-plugin.xml file to use the same URl that the Status Server uses.
 * This presumes the Status Server and this server are on the same machine.
 * 
            String path = System.getProperty("conf.dir");
            
            // Find the URL the status server is using
            File s_propertiesFile = new File(path + "/status-plugin.xml");

            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            DocumentBuilder db = dbf.newDocumentBuilder();
            Document doc = db.parse(s_propertiesFile);
            NodeList list = doc.getElementsByTagName("voicemail-cgi-url");
                
            Node firstNode = list.item(0);
            if(firstNode.getNodeType() == Node.ELEMENT_NODE){
 
                NodeList textFNlist = ((Element)firstNode).getChildNodes();
                mwiStatusURL = ((Node)textFNlist.item(0)).getNodeValue().trim();                                                                             
            }
*/
   
            // For now, just hardcode this.  We cannot steal the CGI script's port as
            // that comes from Apache and it is using that port for other things besides
            // just MWI.
            // TODO Replace with configuration, and change status-plugin.xml to point to here
            mwiStatusURL = "http://localhost:8085/mwi";
            
            url = new URL (mwiStatusURL);              
            

            // Start up jetty
            HttpServer server = new HttpServer();

            // Bind the port on all interfaces
            // TODO HTTPS support
            server.addListener(":" + url.getPort());

            HttpContext httpContext = new HttpContext();
            httpContext.setContextPath("/");

            // Setup the servlet to call the Mwistatus class when that URL is fetched
            ServletHandler servletHandler = new ServletHandler();
            servletHandler.addServlet("mwistatus", url.getPath(), Mwistatus.class.getName());
            httpContext.addHandler(servletHandler);

            server.addContext(httpContext);
            
            // Start it up.
            LOG.info(String.format("Starting MWI servlet on %s", url.toExternalForm()));
            server.start();
        } catch (Exception e) {
            e.printStackTrace(); 
        }
    }

}
