/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.faxrx;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.Properties;

import javax.activation.DataHandler;
import javax.activation.DataSource;
import javax.activation.FileDataSource;
import javax.mail.MessagingException;
import javax.mail.Multipart;
import javax.mail.Part;
import javax.mail.Session;
import javax.mail.Transport;
import javax.mail.internet.AddressException;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeBodyPart;
import javax.mail.internet.MimeMessage;
import javax.mail.internet.MimeMultipart;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.FaxReceive;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.User.EmailFormats;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxivr.SipxIvrApp;
import org.sipfoundry.sipxivr.rest.RemoteRequest;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

public class FaxRx extends SipxIvrApp implements ApplicationContextAware {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    // Global store for AutoAttendant resource bundles keyed by locale

    private String m_sendImUrl;
    private ValidUsers m_validUsers;
    private ApplicationContext m_appContext;

    @Override
    public void run() {
        // run linger only for fax, otherwise we end up with hunged FS session
        FaxRxEslRequestController controller = (FaxRxEslRequestController) getEslRequestController();
        controller.linger();
        // Wait a bit
        controller.sleep(2000);

        receive(controller);
    }

    private void sendIM(User user, String instantMsg) {
        URL sendIMUrl;

        if (m_sendImUrl == null) {
            return;
        }

        try {
            sendIMUrl = new URL(m_sendImUrl + "/" + user.getUserName() + "/sendFaxReceiveIM");
            RemoteRequest rr = new RemoteRequest(sendIMUrl, "text/plain", instantMsg);
            if (!rr.http()) {
                LOG.error("faxrx::sendIM Trouble with RemoteRequest " + rr.getResponse());
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void sendEmail(String emailAddr, File tiffFile, String faxSubject) {

        if (emailAddr == null) {
            return;
        }

        Properties props = System.getProperties();
        props.put("mail.smtp.host", "localhost");
        props.put("mail.smtp.user", "postmaster");
        Session session = Session.getDefaultInstance(props, null);

        MimeMessage message = new MimeMessage(session);

        try {
            message.addRecipient(MimeMessage.RecipientType.TO, new InternetAddress(emailAddr));
            String senderName = m_appContext.getMessage("SenderName", null, null);
            String senderMailTo = m_appContext.getMessage("SenderMailto", null, null);
            message.setFrom(new InternetAddress(String.format("%s <%s>", senderName, senderMailTo)));
            message.setSubject(faxSubject, "UTF-8");

            MimeBodyPart faxBodyPart = new MimeBodyPart();

            DataSource dataSource = new FileDataSource(tiffFile) {
                public String getContentType() {
                    return "image/tiff";
                }
            };

            faxBodyPart.setDataHandler(new DataHandler(dataSource));
            faxBodyPart.setFileName("fax-message.tiff");
            faxBodyPart.setHeader("Content-Transfer-Encoding", "base64");
            faxBodyPart.setDisposition(Part.ATTACHMENT);

            Multipart mpmixed = new MimeMultipart();
            mpmixed.addBodyPart(faxBodyPart);
            message.setContent(mpmixed);

            Transport.send(message);

        } catch (AddressException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (MessagingException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    private void receive(FaxRxEslRequestController controller) {
        File faxPathName = null;
        FaxReceive faxReceive = null;
        String faxInfo;
        String mailboxId = controller.getMailboxId();
        String locale = controller.getLocaleString();

        LOG.info("faxrx::Starting mailbox (" + mailboxId + ") in locale " + locale);

        User user = m_validUsers.getUser(mailboxId);
        if (user == null) {
            LOG.error("FaxReceive: no user found for mailbox " + mailboxId);
            return;
        }

        if (locale != null) {
            user.setLocale(controller.getLocale(locale));
        }

        try {
            faxPathName = File.createTempFile("fax_", ".tiff");
            controller.invokeSet("fax_enable_t38_request", "true");
            controller.invokeSet("fax_enable_t38", "true");
            faxReceive = controller.receiveFax(faxPathName.getAbsolutePath());

        } catch (IOException e) {
            e.printStackTrace();
            return;
        }

        finally {

            // construct a reasonable faxInfo string to be used as part of the email
            // subject and instant message. Send email even if faxReceive.rxSuccess() false
            // as there could be incomplete faxes - act as a fax machine
            String name = null;
            String number = null;

            if (faxReceive.getRemoteStationId() != null) {
                name = faxReceive.getRemoteStationId();
            } else {
                if (!controller.getChannelCallerIdName().equals("unknown")) {
                    name = controller.getChannelCallerIdName();
                }
            }

            if (!controller.getChannelCallerIdNumber().equals("0000000000")) {
                number = controller.getChannelCallerIdNumber();
            }

            faxInfo = faxReceive.faxTotalPages() + " "
                    + m_appContext.getMessage("page_fax_from", null, "page fax from", user.getLocale()) + " ";
            if (name != null) {
                faxInfo += name + " ";
            }

            if (number != null) {
                faxInfo += "(" + number + ")";
            }

            if (name == null && number == null) {
                faxInfo += m_appContext.getMessage("an_unknown_sender", null, "an unknown sender", user.getLocale());
            }

            // need to send to at least one email address
            boolean sent = false;
            String faxSubject = m_appContext.getMessage("Your", null, "Your", user.getLocale()) + " " + faxInfo;

            if (user.getEmailFormat() != EmailFormats.FORMAT_NONE) {
                sendEmail(user.getEmailAddress(), faxPathName, faxSubject);
                sent = true;
            }

            if (user.getAltEmailFormat() != EmailFormats.FORMAT_NONE) {
                sendEmail(user.getAltEmailAddress(), faxPathName, faxSubject);
                sent = true;
            }

            // need to send to at least one email address so let's be more aggressive

            if (!sent) {
                if (user.getEmailAddress() != null) {
                    sendEmail(user.getEmailAddress(), faxPathName, faxSubject);
                    sent = true;
                }
            }

            if (!sent) {
                // need to send to at least one email address so let's be even more aggressive
                if (user.getAltEmailAddress() != null) {
                    sendEmail(user.getAltEmailAddress(), faxPathName, faxSubject);
                } else {
                    // didn't send anywhere !!
                    LOG.error("Fax Receive: No email address for user " + user.getUserName());
                }
            }

            if (faxReceive.rxSuccess()) {
                LOG.debug("Fax received successfully " + faxInfo);
                sendIM(user, m_appContext.getMessage("You_received_a", null, "You received a", user.getLocale())
                        + " " + faxInfo + ".");
            } else {
                LOG.error("Fax receive failed from " + controller.getChannelCallerIdNumber() + ". Error text: "
                        + faxReceive.getResultText() + ". Error code: " + faxReceive.getResultCode());
                sendIM(user,
                        m_appContext.getMessage("You_received_an_incomplete", null, "You received an incomplete",
                                user.getLocale()) + " " + faxInfo + ".");
            }

            faxPathName.delete();
        }
    }

    public void setSendImUrl(String url) {
        m_sendImUrl = url;
    }

    public void setValidUsers(ValidUsers validUsers) {
        m_validUsers = validUsers;
    }

    @Override
    public void setApplicationContext(ApplicationContext context) {
        m_appContext = context;
    }
}
