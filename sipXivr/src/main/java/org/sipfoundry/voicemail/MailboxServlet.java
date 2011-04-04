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
import java.util.List;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.commons.util.SipUriUtil;
import org.sipfoundry.sipxivr.Mailbox;

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
 *      /messages GET returns all voicemail messages
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


        User user = ValidUsers.INSTANCE.getUser(mailboxString);
        // only superadmin and mailbox owner can access this service
        // TODO allow all admin user to access it
        String authenticatedUserName = request.getUserPrincipal().getName();
        if (!authenticatedUserName.equals(user.getUserName())) {
            response.sendError(403); // Send 403 Forbidden
            return;
        }
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

                        if (action.equals("delete")) {
                            if(method.equals(METHOD_PUT)) {
                                VmMessage msg = messages.getMessage(messageId);
                                if(msg != null) {
                                    messages.deleteMessage(msg);
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

                } else if (context.equals("messages")) {
                    if (method.equals(METHOD_GET)) {
                        response.setContentType("text/xml");
                        pw.write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
                        pw.write("<messages>\n");
                        listMessages(messages.getInbox(), "inbox", pw);
                        listMessages(messages.getSaved(), "saved", pw);
                        listMessages(messages.getDeleted(), "deleted", pw);
                        pw.write("</messages>");
                    } else {
                        response.sendError(405);
                    }
                }
                else {
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

    private void listMessages(List<VmMessage> messages, String folder, PrintWriter pw) {
        String author = null;
        for (VmMessage message : messages) {
            author =  SipUriUtil.extractUserName(message.getMessageDescriptor().getFromUri().replace('+', ' '));
            pw.format("<message id=\"%s\" heard=\"%s\" urgent=\"%s\" folder=\"%s\" duration=\"%s\" received=\"%s\" author=\"%s\"/>\n",
                    message.getMessageId(), !message.isUnHeard(), message.isUrgent(), folder, message.getDuration(),
                    message.getTimestamp(), author);
        }
    }

}
