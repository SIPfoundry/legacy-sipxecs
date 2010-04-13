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
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.mail.MailException;
import org.springframework.mail.SimpleMailMessage;
import org.springframework.mail.javamail.JavaMailSender;
import org.springframework.mail.javamail.MimeMessageHelper;

public class MailSenderContextImpl implements MailSenderContext {
    private static final Log LOG = LogFactory.getLog(MailSenderContextImpl.class);

    private JavaMailSender m_mailSender;

    private DomainManager m_domainManager;

    public void sendMail(String to, String from, String subject, String body) {
        SimpleMailMessage msg = new SimpleMailMessage();
        msg.setTo(getFullAddress(from));
        msg.setFrom(getFullAddress(from));
        msg.setSubject(subject);
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
            helper.setTo(getFullAddress(to));
            helper.setFrom(getFullAddress(from));
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

    /**
     * Appends SIP domain name to addresses that do not have domain names
     *
     * @param address
     * @return
     */
    public String getFullAddress(String address) {
        if (address.contains("@")) {
            return address;
        }
        return String.format("%s@%s", address, m_domainManager.getDomain().getName());
    }

    @Required
    public void setMailSender(JavaMailSender mailSender) {
        m_mailSender = mailSender;
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
