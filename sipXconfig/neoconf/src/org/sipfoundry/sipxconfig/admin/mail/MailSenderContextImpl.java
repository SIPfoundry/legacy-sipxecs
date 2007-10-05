/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.mail;

import java.io.File;
import java.util.Date;
import java.util.Properties;

import javax.activation.DataHandler;
import javax.activation.FileDataSource;
import javax.mail.Message;
import javax.mail.MessagingException;
import javax.mail.Multipart;
import javax.mail.Session;
import javax.mail.Transport;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeBodyPart;
import javax.mail.internet.MimeMessage;
import javax.mail.internet.MimeMultipart;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class MailSenderContextImpl implements MailSenderContext {
    private static final Log LOG = LogFactory.getLog(MailSenderContextImpl.class);

    private static final String SMTP_SENDER = "localhost";

    private static final String MAIL_SMTP_HOST = "mail.smtp.host";

    private Message createMessage(String to, String cc, String bcc, String from, String subject)
        throws MessagingException {
        Properties props = System.getProperties();

        props.put(MAIL_SMTP_HOST, SMTP_SENDER);
        Session session = Session.getDefaultInstance(props, null);

        Message msg = new MimeMessage(session);

        if (from == null) {
            throw new MessagingException("From field must be specified");
        }
        msg.setFrom(new InternetAddress(from));
        if (to == null) {
            throw new MessagingException("To field must be specified");
        }

        msg.setRecipients(Message.RecipientType.TO, InternetAddress.parse(to, false));

        if (cc != null) {
            msg.setRecipients(Message.RecipientType.CC, InternetAddress.parse(cc, false));
        }
        if (bcc != null) {
            msg.setRecipients(Message.RecipientType.BCC, InternetAddress.parse(bcc, false));
        }

        msg.setSubject(subject);

        return msg;

    }

    public void sendMail(String to, String cc, String bcc, String from, String subject,
            String body) {
        try {
            Message msg = createMessage(to, cc, bcc, from, subject);
            msg.setText(body);
            Transport.send(msg);
            msg.setSentDate(new Date());
        } catch (MessagingException ex) {
            LOG.error(ex);
        }
    }

    public void sendMail(String to, String cc, String bcc, String from, String subject,
            String body, File[] files) {
        try {
            if (files == null || files.length == 0) {
                // only for Voicemail backup
                sendMail(to, cc, bcc, from, subject, body);
            } else {
                Message msg = createMessage(to, cc, bcc, from, subject);

                MimeBodyPart mbp1 = new MimeBodyPart();
                mbp1.setText(body);

                MimeBodyPart[] mbp2 = new MimeBodyPart[files.length];
                Multipart mp = new MimeMultipart();
                mp.addBodyPart(mbp1);

                for (int i = 0; i < files.length; i++) {
                    FileDataSource fds = new FileDataSource(files[i]);
                    mbp2[i] = new MimeBodyPart();
                    mbp2[i].setDataHandler(new DataHandler(fds));
                    mbp2[i].setFileName(fds.getName());
                    mp.addBodyPart(mbp2[i]);
                }

                msg.setContent(mp);
                msg.setSentDate(new Date());
                Transport.send(msg);
            }
        } catch (MessagingException ex) {
            LOG.error(ex);
        }
    }

}
