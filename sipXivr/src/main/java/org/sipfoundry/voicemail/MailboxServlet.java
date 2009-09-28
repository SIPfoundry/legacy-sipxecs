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
import java.io.PrintWriter;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.User;
import org.sipfoundry.sipxivr.ValidUsersXML;

/**
 * A RESTful interface to the mailbox messages
 * 
 * Only two services at the moment:
 *    Mark a message read
 *    Update the MWI status
 *    
 * Prefix is /mailbox/*
 * Paths are
 * /{mailbox}/
 *      /mwi   PUT (no data) updates the MWI for this mailbox (i.e. tells the status server to update the MWI status of devices
 *             GET returns the MWI status for this mailbox
 *      /messages/
 *          /{messageId}
 *              /heard   PUT (no data) Marks the message heard (and updates MWI)
 *                       GET returns message heard status
 *                       DELETE Marks the message unheard (and updates MWI)
 */
public class MailboxServlet extends HttpServlet {
    private static final long serialVersionUID = 1L;
    private static final String METHOD_DELETE = "DELETE";
    private static final String METHOD_GET = "GET";
    private static final String METHOD_PUT = "PUT";
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    public void doPut(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        doIt(request, response);
    }

    public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        doIt(request, response);
    }

    public void doDelete(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        doIt(request, response);
    }

    public void doIt(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        String method = request.getMethod().toUpperCase();

        // For (minimal) security, check the request is arriving from THIS machine
        // TODO (change this once using https and authentication of remote party)
        if (!request.getLocalAddr().equals(request.getRemoteAddr())) {
            response.sendError(403); // Send 403 Forbidden
            return;
        }

        String pathInfo = request.getPathInfo();
        String[] subDirs = pathInfo.split("/");
        if (subDirs.length < 3) {
            response.sendError(404);    // path not found
            return;
        }
        
        // The first element is empty (the leading slash)
        // The second element is the mailbox
        String mailboxString = subDirs[1];
        // The third element is the "context" (either mwi, message)
        String context = subDirs[2];

        
        // Load the list of valid users 
        // (it is static, so don't worry about sucking it in each time, it'll only 
        // be re-read if it has changed on disk)
        ValidUsersXML validUsers = ValidUsersXML.update(true);
        User user = validUsers.isValidUser(mailboxString);
        if (user != null) {
            PrintWriter pw = response.getWriter();
            LOG.info(String.format("MailboxServlet::doIt %s %s", method, pathInfo));
            // determine the message counts for the mailbox
            // (Okay, worry about this one.  It walks the mailstore directories counting .xml and .sta files.)
            Mailbox mailbox = new Mailbox(user);
            Messages messages = Messages.newMessages(mailbox);
            synchronized (messages) {
                if (context.equals("message")) {
                    if (subDirs.length >= 5) {
                        String messageId = subDirs[3];
                        String action = subDirs[4];
                        if (action.equals("heard")) {
                            if (method.equals(METHOD_DELETE)) {
                                messages.markMessageUnheard(messageId, true);
                            } else if (method.equals(METHOD_PUT)) {
                                messages.markMessageHeard(messageId, true);
                            }
                            else if (method.equals(METHOD_GET)) {
                                VmMessage msg = messages.getMessage(messageId);
                                response.setContentType("text/xml");
                                pw.write("<message>\n") ;
                                pw.format("  <heard>%s</heard>\n", msg.isUnHeard() ? "false" : "true" );
                                pw.write("</message>\n") ;
                            }
                        }
                    } else {
                        response.sendError(400, "messageId missing");
                    }
                } else if (context.equals("mwi")) {
                    if (method.equals(METHOD_PUT)) {
                        Mwi.sendMWI(mailbox, messages);
                    } else if (method.equals(METHOD_GET)) {
                        response.setContentType(Mwi.MessageSummaryContentType);
                        pw.write(Mwi.formatRFC3842(messages));
                    } else {
                        response.sendError(405);
                    }
                } else {
                    response.sendError(400, "context not understood");
                }
            }
            Messages.releaseMessages(messages);
            pw.close();
        } else {
            response.sendError(404, "Mailbox not found");
            LOG.info(String.format("MailboxServlet::doIt %s not found", mailboxString));
        }

    }

}
