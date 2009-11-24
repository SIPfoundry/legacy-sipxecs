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
import java.io.StringWriter;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.bind.JAXBContext;
import javax.xml.bind.Marshaller;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.sipxivr.ActiveGreeting;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.MailboxPreferences;

/**
 * A RESTful interface to the mailbox messages
 * 
 * three services at the moment:
 *    Mark a message read
 *    Update the MWI status
 *    Get the FS channel UID for the current call answering session for the mailbox
 *    Get/Set a user's active greeting type
 *    
 * Prefix is /mailbox/*
 * Paths are
 * /{mailbox}/
 *      /mwi   PUT (no data) updates the MWI for this mailbox (i.e. tells the status server to update the MWI status of devices
 *             GET returns the MWI status for this mailbox
 *      /uuid  GET returns the FS channel UUID for a current call answering session for the mailbox       
 *      /messages/
 *          /{messageId}
 *              /heard   PUT (no data) Marks the message heard (and updates MWI)
 *                       GET returns message heard status
 *                       DELETE Marks the message unheard (and updates MWI)
 *      /preferences/
 *          /activegreeting PUT sets the active greeting (body is fragment <activegreeting>{value}</activegreeting>)
 *                             {value} is one of none, standard, outofoffice, extendedabsence
 *                          GET returns the active greeting (returns fragment above)
 *                          DELETE sets the active greeting to "none"
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
        ValidUsersXML validUsers = null;
        try {
            validUsers = ValidUsersXML.update(LOG, true);
        } catch (Exception e) {
            response.sendError(500, "Cannot read validusers.xml");
            return;
        }
        User user = validUsers.getUser(mailboxString);
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
                                if (msg != null) {
                                    response.setContentType("text/xml");
                                    pw.format("<heard>%s</heard>\n", msg.isUnHeard() ? "false" : "true" );
                                } else {
                                    response.sendError(404, "messageId not found");
                                }
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
                        String accountUrl = "sip:" + user.getIdentity();
                        pw.write(Mwi.formatRFC3842(messages, accountUrl));
                    } else {
                        response.sendError(405);
                    }
                } else if (context.equals("uuid")) {
                    response.setContentType("text/xml");
                    String uuid = Deposit.getChannelUUID(user);
                    if(uuid == null) {
                        pw.write("<uuid></uuid>\n") ;
                    } else {
                        pw.format("<uuid>%s</uuid>\n", uuid);
                    }
                } else if (context.equals("preferences")) {
                    if (subDirs.length >= 4) {
                        String whichPreference = subDirs[3];
                        if (whichPreference.equals("activegreeting")) {
                            MailboxPreferences prefs = mailbox.getMailboxPreferences();
                            if (method.equals(METHOD_DELETE)) {
                                prefs.setActiveGreeting(new ActiveGreeting());
                                mailbox.writeMailboxPreferences();
                            } else if (method.equals(METHOD_PUT)) {
                                try {
                                    JAXBContext jc = JAXBContext.newInstance(ActiveGreeting.class);
                                    ActiveGreeting ag = (ActiveGreeting) jc.createUnmarshaller().unmarshal(
                                            request.getReader());
                                    prefs.setActiveGreeting(ag);
                                    mailbox.writeMailboxPreferences();
                                } catch (Exception e) {
                                    //e.printStackTrace(pw);
                                    response.sendError(400, e.getMessage());
                                }
                            }
                            else if (method.equals(METHOD_GET)) {
                                response.setContentType("text/xml");
                                try {
                                    JAXBContext jc = JAXBContext.newInstance(ActiveGreeting.class);
                                    ActiveGreeting ag = prefs.getActiveGreeting();
                                    Marshaller m = jc.createMarshaller();
                                    m.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT,new Boolean(true));
                                    m.setProperty(Marshaller.JAXB_FRAGMENT, new Boolean(true));
                                    StringWriter xml = new StringWriter();
                                    m.marshal(ag, xml);
                                    pw.write(xml.toString());
                                    pw.write("\n");
                                } catch (Exception e) {
                                    e.printStackTrace(pw);
                                    response.sendError(400, e.getMessage());
                                }
                            }
                        } else {
                            response.sendError(400, "preference not understood");
                        }
                    } else {
                        response.sendError(400, "preference selection missing");
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
