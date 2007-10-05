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

import javax.mail.MessagingException;
import javax.mail.internet.MimeMessage;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.mail.MailException;
import org.springframework.mail.SimpleMailMessage;
import org.springframework.mail.javamail.JavaMailSender;
import org.springframework.mail.javamail.MimeMessageHelper;

public class MailSenderContextImpl implements MailSenderContext {
    private static final Log LOG = LogFactory.getLog(MailSenderContextImpl.class);

    private JavaMailSender m_mailSender;

    public void sendMail(String to, String from, String subject, String body) {        
        SimpleMailMessage msg1 = new SimpleMailMessage();
        msg1.setFrom(from);
        msg1.setTo(to);
        msg1.setSubject(subject);
        SimpleMailMessage msg = msg1;
        msg.setText(body);
        msg.setSentDate(new Date());
        try {
            m_mailSender.send(msg);
        } catch (MailException e) {
            LOG.error(e);
        }
        
    }

    public void sendMail(String to, String from, String subject, String body, File... files) {
        if (files == null || files.length == 0) {
            sendMail(to, from, subject, body);
            return;
        }
        try {
            MimeMessage msg = m_mailSender.createMimeMessage();
            MimeMessageHelper helper = new MimeMessageHelper(msg, true);
            helper.setTo(to);
            helper.setFrom(from);
            helper.setSubject(subject);
            helper.setText(body);

            for (File f : files) {
                helper.addAttachment(f.getName(), f);
            }
            m_mailSender.send(msg);
        } catch (MailException e) {
            LOG.error(e);
        } catch (MessagingException e) {
            LOG.error(e);
        }
    }

    public void setMailSender(JavaMailSender mailSender) {
        m_mailSender = mailSender;
    }
}
