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
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxivr.rest.SipxIvrServletHandler;
import org.sipfoundry.voicemail.mailbox.MailboxManager;

/**
 * Return the status (heard/unheard messages) of a mailbox using a simple HTTP request.
 * 
 * Replaces the existing CGI script used by the Status Server process.
 *
 * Given a request like:
 *    http://<this server>/mwi?identity=user@domain
 *    
 * A response like this is returned:
 *    HTTP/1.1 200 OK
 *    Date: Wed, 23 Sep 2009 19:08:59 GMT
 *    Server: Jetty/5.1.4 (Linux/2.6.27.24-170.2.68.fc10.i686 i386 java/1.6.0_11
 *    Content-Type: application/simple-message-summary
 *    Content-Length: 51
 *
 *    Messages-Waiting: yes
 *    Voice-Message: 1/2 (0/0)
 *
 */
public class Mwistatus extends HttpServlet {
    private static final long serialVersionUID = 2609976094457923038L;
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    
    public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        ValidUsers validUsers = (ValidUsers) request.getAttribute(SipxIvrServletHandler.VALID_USERS_ATTR);
        MailboxManager mailboxManager = (MailboxManager) request.getAttribute(SipxIvrServletHandler.MAILBOX_MANAGER);
        response.setContentType(Mwi.MessageSummaryContentType);
        // Use the OutputStream rather than the PrintWriter as this will cause Jetty
        // To NOT set the charset= parameter on the content type, which breaks
        // The status server doing this request.
        // The price is we aren't specifying the encoding of this message (sigh).
        OutputStream os = response.getOutputStream();
        String rfc3842 = Mwi.formatRFC3842(0, 0, 0, 0, ""); // Default to nothing

        // read the query string
        String idUri = request.getParameter("identity");
        
        User user = validUsers.getUser(ValidUsers.getUserPart(idUri));
        if (user != null) {
            // determine the message counts for the mailbox
            // (Okay, worry about this one.  It walks the mailstore directories counting .xml and .sta files.)
            String accountUrl = "sip:" + user.getIdentity();
            rfc3842 = Mwi.formatRFC3842(mailboxManager.getMailboxDetails(user.getUserName()), accountUrl);
            LOG.info(String.format("Mwistatus::doGet %s", idUri));
        } else {
            // Just lie and give no messages
            LOG.info(String.format("Mwistatus::doGet %s not found", idUri));
        }

        // Just dump the bytes of the string.  No character encoding or nothing.
        os.write(rfc3842.getBytes());
        os.close();
    }

}
