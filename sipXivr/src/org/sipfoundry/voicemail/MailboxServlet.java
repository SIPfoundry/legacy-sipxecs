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
import java.security.Principal;
import java.util.List;
import java.util.Map;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.commons.util.SipUriUtil;
import org.sipfoundry.sipxivr.SipxIvrConfiguration;
import org.sipfoundry.sipxivr.rest.SipxIvrServletHandler;
import org.sipfoundry.voicemail.mailbox.Folder;
import org.sipfoundry.voicemail.mailbox.MailboxDetails;
import org.sipfoundry.voicemail.mailbox.MailboxManager;
import org.sipfoundry.voicemail.mailbox.MessageDescriptor;
import org.sipfoundry.voicemail.mailbox.MessageNotFoundException;
import org.sipfoundry.voicemail.mailbox.VmMessage;

/**
 * A RESTful interface to the mailbox messages
 *
 * three services at the moment: Mark a message read Update the MWI status Get the FS channel UID
 * for the current call answering session for the mailbox Get/Set a user's active greeting type
 *
 * Prefix is /mailbox/* Paths are /{mailbox}/ /mwi PUT (no data) updates the MWI for this mailbox
 * (i.e. tells the status server to update the MWI status of devices GET returns the MWI status
 * for this mailbox /uuid GET returns the FS channel UUID for a current call answering session for
 * the mailbox /messages GET returns all voicemail messages /messages/ /{messageId} /heard PUT (no
 * data) Marks the message heard (and updates MWI) GET returns message heard status DELETE Marks
 * the message unheard (and updates MWI) /preferences/ /activegreeting PUT sets the active
 * greeting (body is fragment <activegreeting>{value}</activegreeting>) {value} is one of none,
 * standard, outofoffice, extendedabsence GET returns the active greeting (returns fragment above)
 * DELETE sets the active greeting to "none"
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

    public void doDelete(HttpServletRequest request, HttpServletResponse response) throws ServletException,
            IOException {
        doIt(request, response);
    }

    public void doIt(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        SipxIvrConfiguration ivrConfig = (SipxIvrConfiguration) request
                .getAttribute(SipxIvrServletHandler.IVR_CONFIG_ATTR);
        ValidUsers validUsers = (ValidUsers) request.getAttribute(SipxIvrServletHandler.VALID_USERS_ATTR);
        Map<String, String> depositMap = (Map<String, String>) request
                .getAttribute(SipxIvrServletHandler.DEPOSIT_MAP_ATTR);
        MailboxManager mailboxManager = (MailboxManager) request.getAttribute(SipxIvrServletHandler.MAILBOX_MANAGER);
        Mwi mwiManager = (Mwi) request.getAttribute(SipxIvrServletHandler.MWI_MANAGER);

        String method = request.getMethod().toUpperCase();

        String pathInfo = request.getPathInfo();
        String[] subDirs = pathInfo.split("/");
        if (subDirs.length < 3) {
            response.sendError(404); // path not found
            return;
        }

        // The first element is empty (the leading slash)
        // The second element is the mailbox
        String mailboxString = subDirs[1];
        // The third element is the "context" (either mwi, message)
        String context = subDirs[2];

        User user = validUsers.getUser(mailboxString);
        // only superadmin and mailbox owner can access this service
        // TODO allow all admin user to access it

        if (user != null && isForbidden(request, user.getUserName(), request.getLocalPort(), ivrConfig.getHttpPort())) {
            response.sendError(403); // Send 403 Forbidden
            return;
        }

        // delete mailbox could come only from a internal port, when user already deleted from
        // mongo
        if (context.equals("delete")) {
            if (ivrConfig.getHttpPort() == request.getLocalPort()) {
                if (method.equals(METHOD_PUT)) {
                    try {
                        mailboxManager.deleteMailbox(mailboxString);
                    } catch (Exception ex) {
                        response.sendError(500);
                    }
                } else {
                    response.sendError(405);
                }
            }
        } else {
            if (user != null) {
                PrintWriter pw = response.getWriter();
                LOG.info(String.format("MailboxServlet::doIt %s %s", method, pathInfo));
                // determine the message counts for the mailbox
                // (Okay, worry about this one. It walks the mailstore directories counting .xml
                // and
                // .sta files.)
                if (context.equals("message")) {
                    if (subDirs.length >= 5) {
                        String messageId = subDirs[3];
                        String action = subDirs[4];
                        if (action.equals("heard")) {
                            if (method.equals(METHOD_DELETE)) {
                                mailboxManager.markMessageUnheard(user, messageId);
                            } else if (method.equals(METHOD_PUT)) {
                                mailboxManager.markMessageHeard(user, messageId);
                            } else if (method.equals(METHOD_GET)) {
                                try {
                                    response.setContentType("text/xml");
                                    pw.format("<heard>%s</heard>\n",
                                            mailboxManager.isMessageUnHeard(user, messageId) ? "false" : "true");
                                } catch (MessageNotFoundException ex) {
                                    response.sendError(404, "messageId not found");
                                }
                            }
                        }

                        if (action.equals("delete")) {
                            if (method.equals(METHOD_PUT)) {
                                try {
                                    mailboxManager.deleteMessage(user, messageId);
                                } catch (MessageNotFoundException ex) {
                                    response.sendError(404, "messageId not found");
                                }
                            }
                        }

                        if (action.equals("subject")) {
                            if (method.equals(METHOD_PUT)) {
                                try {
                                    String subject = IOUtils.toString(request.getInputStream());
                                    mailboxManager.updateMessageSubject(user, messageId, subject);
                                } catch (MessageNotFoundException ex) {
                                    response.sendError(404, "messageId not found");
                                }
                            }
                        }

                        if (action.equals("move")) {
                            if (method.equals(METHOD_PUT)) {
                                try {
                                    String destinationFolder = subDirs[5];
                                    mailboxManager.moveMessageToFolder(user, messageId, destinationFolder);
                                    MailboxDetails mailbox = mailboxManager.getMailboxDetails(user.getUserName());
                                    mwiManager.sendMWI(user, mailbox);
                                } catch (MessageNotFoundException ex) {
                                    response.sendError(404, "messageId not found");
                                }
                            }
                        }

                    } else {
                        response.sendError(400, "messageId missing");
                    }
                } else if (context.equals("mwi")) {
                    if (method.equals(METHOD_PUT)) {
                        MailboxDetails mailbox = mailboxManager.getMailboxDetails(user.getUserName());
                        mwiManager.sendMWI(user, mailbox);
                    } else if (method.equals(METHOD_GET)) {
                        response.setContentType(Mwi.MessageSummaryContentType);
                        String accountUrl = "sip:" + user.getIdentity();
                        MailboxDetails mailbox = mailboxManager.getMailboxDetails(user.getUserName());
                        pw.write(Mwi.formatRFC3842(mailbox, accountUrl));
                    } else {
                        response.sendError(405);
                    }
                } else if (context.equals("uuid")) {
                    response.setContentType("text/xml");
                    String uuid = depositMap.get(user.getUserName());
                    if (uuid == null) {
                        pw.write("<uuid></uuid>\n");
                    } else {
                        pw.format("<uuid>%s</uuid>\n", uuid);
                    }

                } else if (context.equals("rename")) {
                    if (subDirs.length >= 4) {
                        String oldMailbox = subDirs[3];
                        if (method.equals(METHOD_PUT)) {
                            try {
                                mailboxManager.renameMailbox(user, oldMailbox);
                            } catch (Exception ex) {
                                response.sendError(500);
                            }
                        } else {
                            response.sendError(405);
                        }
                    } else {
                        response.sendError(400, "destination missing");
                    }

                } else if (context.equals("messages")) {
                    if (method.equals(METHOD_GET)) {
                        response.setContentType("text/xml");
                        pw.write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
                        pw.write("<messages>\n");
                        List<VmMessage> inboxMessages = mailboxManager.getMessages(user.getUserName(), Folder.INBOX);
                        List<VmMessage> savedMessages = mailboxManager.getMessages(user.getUserName(), Folder.SAVED);
                        List<VmMessage> deletedMessages = mailboxManager.getMessages(user.getUserName(),
                                Folder.DELETED);
                        listMessages(inboxMessages, "inbox", pw);
                        listMessages(savedMessages, "saved", pw);
                        listMessages(deletedMessages, "deleted", pw);
                        pw.write("</messages>");
                    } else {
                        response.sendError(405);
                    }
                } else if (context.equals("inbox")) {
                    if (method.equals(METHOD_GET)) {
                        response.setContentType("text/xml");
                        pw.write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
                        pw.write("<messages>\n");
                        if (subDirs.length >= 4) {
                            String messageId = subDirs[3];
                            try {
                                listFullMessage(mailboxManager.getVmMessage(user.getUserName(), Folder.INBOX,
                                        messageId, false), "inbox", pw);
                            } catch (MessageNotFoundException ex) {
                                response.sendError(404, "messageId not found");
                            }
                        } else {
                            listFullMessages(mailboxManager.getMessages(user.getUserName(), Folder.INBOX), "inbox",
                                    pw);
                        }
                        pw.write("</messages>");
                    } else {
                        response.sendError(405);
                    }
                } else if (context.equals("saved")) {
                    if (method.equals(METHOD_GET)) {
                        response.setContentType("text/xml");
                        pw.write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
                        pw.write("<messages>\n");
                        if (subDirs.length >= 4) {
                            String messageId = subDirs[3];
                            try {
                                listFullMessage(mailboxManager.getVmMessage(user.getUserName(), Folder.SAVED,
                                        messageId, false), "saved", pw);
                            } catch (MessageNotFoundException ex) {
                                response.sendError(404, "messageId not found");
                            }
                        } else {
                            listFullMessages(mailboxManager.getMessages(user.getUserName(), Folder.SAVED), "saved",
                                    pw);
                        }
                        pw.write("</messages>");
                    } else {
                        response.sendError(405);
                    }
                } else if (context.equals("deleted")) {
                    if (method.equals(METHOD_GET)) {
                        response.setContentType("text/xml");
                        pw.write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
                        pw.write("<messages>\n");
                        if (subDirs.length >= 4) {
                            String messageId = subDirs[3];
                            try {
                                listFullMessage(mailboxManager.getVmMessage(user.getUserName(), Folder.DELETED,
                                        messageId, false), "deleted", pw);
                            } catch (MessageNotFoundException ex) {
                                response.sendError(404, "messageId not found");
                            }
                        } else {
                            listFullMessages(mailboxManager.getMessages(user.getUserName(), Folder.DELETED),
                                    "deleted", pw);
                        }
                        pw.write("</messages>");
                    } else {
                        response.sendError(405);
                    }
                } else if (context.equals("conference")) {
                    if (method.equals(METHOD_GET)) {
                        response.setContentType("text/xml");
                        pw.write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
                        pw.write("<messages>\n");
                        if (subDirs.length >= 4) {
                            String messageId = subDirs[3];
                            try {
                                listFullMessage(mailboxManager.getVmMessage(user.getUserName(), Folder.CONFERENCE,
                                        messageId, false), "conference", pw);
                            } catch (MessageNotFoundException ex) {
                                response.sendError(404, "messageId not found");
                            }
                        } else {
                            listFullMessages(mailboxManager.getMessages(user.getUserName(), Folder.CONFERENCE),
                                    "conference", pw);
                        }
                        pw.write("</messages>");
                    } else {
                        response.sendError(405);
                    }
                } else {
                    response.sendError(400, "context not understood");
                }
                pw.close();
            } else {
                response.sendError(404, "Mailbox not found");
                LOG.info(String.format("MailboxServlet::doIt %s not found", mailboxString));
            }
        }

    }

    private boolean isForbidden(HttpServletRequest request, String userName, int requestPort, int port) {
        if (requestPort != port) {
            Principal principal = request.getUserPrincipal();
            String authenticatedUserName = (principal == null) ? null : principal.getName();

            return authenticatedUserName != null && (!authenticatedUserName.equals(userName) && !authenticatedUserName.equals("superadmin"));
        } else {
            return false;
        }
    }

    private void listMessages(List<VmMessage> messages, String folder, PrintWriter pw) {
        String author = null;
        for (VmMessage message : messages) {
            MessageDescriptor descriptor = message.getDescriptor();
            author = SipUriUtil.extractUserName(descriptor.getFromUri().replace('+', ' '));
            pw.format(
                    "<message id=\"%s\" heard=\"%s\" urgent=\"%s\" folder=\"%s\" duration=\"%s\" received=\"%s\" author=\"%s\"/>\n",
                    message.getMessageId(), !message.isUnHeard(), message.isUrgent(), folder,
                    descriptor.getDurationSecsLong(), descriptor.getTimeStampDate().getTime(), author);
        }
    }

    private void listFullMessages(List<VmMessage> messages, String folder, PrintWriter pw) {
        for (VmMessage message : messages) {
            listFullMessage(message, folder, pw);
        }
    }

    private void listFullMessage(VmMessage message, String folder, PrintWriter pw) {
        MessageDescriptor descriptor = message.getDescriptor();
        String author = SipUriUtil.extractUserName(descriptor.getFromUri().replace('+', ' '));
        pw.format(
                "<message id=\"%s\" heard=\"%s\" urgent=\"%s\" folder=\"%s\" duration=\"%s\" received=\"%s\" author=\"%s\" subject=\"%s\"/>\n",
                message.getMessageId(), !message.isUnHeard(), message.isUrgent(), folder,
                descriptor.getDurationSecsLong(), descriptor.getTimeStampDate().getTime(), author,
                descriptor.getSubject());
    }

}
