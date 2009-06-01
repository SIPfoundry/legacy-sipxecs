/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */

package org.sipfoundry.voicemail;

import java.io.File;
import java.util.Properties;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

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
import org.sipfoundry.sipxivr.Configuration;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.MailboxPreferences;

public class Emailer {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private static Emailer s_me;
    boolean m_justTesting;
    ExecutorService m_es;
    Session m_session;
    Configuration m_ivrConfig;

    private Emailer() {}
    
    /**
     * Queue up sending the VmMessage as an e-mail to the addresses specified in the mailbox
     * 
     * @param loc
     * @param mailbox
     * @param vmessage
     */
    public static void queueVm2Email(Mailbox mailbox, VmMessage vmessage) {
        if (isJustTesting()) {
            return;
        }
        
        MailboxPreferences mbPrefs = mailbox.getMailboxPreferences();
        String to = mbPrefs.getEmailAddress();
        if (to == null) {
            to = mbPrefs.getAlternateEmailAddress();
        }
        if (to != null) {
            LOG.info("Emailer::queueVm2Email queuing e-mail for "+to);
            BackgroundMailer bm = getEmailer().new BackgroundMailer(mailbox, vmessage);
            submit(bm);
        }
    }

    /**
     * The Runnable class that builds and sends the e-mail
     */
    class BackgroundMailer implements Runnable {
        Mailbox m_mailbox;
        VmMessage m_vmessage;

        BackgroundMailer(Mailbox mailbox, VmMessage vmessage) {
            m_mailbox = mailbox;
            m_vmessage = vmessage;
        }
        
        /**
         * Build up the MIME multipart formatted e-mail
         * 
         * @param attachWav Attach the wav file as part of the message
         * @return
         * @throws AddressException
         * @throws MessagingException
         */
        MimeMessage buildMessage(boolean attachWav) throws AddressException, MessagingException {
            EmailFormatter emf = new EmailFormatter(m_ivrConfig, m_mailbox, m_vmessage);
            MimeMessage message = new MimeMessage(m_session);
            message.setFrom(new InternetAddress(emf.getSender()));  

            message.setSubject(emf.getSubject());
            
            // Create an "mulipart/alternative" part with text and html alternatives
            Multipart mpalt = new MimeMultipart("alternative");

            // Add the text part of the message first
            MimeBodyPart textPart = new MimeBodyPart();
            textPart.setText(emf.getTextBody());
            mpalt.addBodyPart(textPart);

            // Add the HTML part of the message
            MimeBodyPart htmlpart = new MimeBodyPart();
            htmlpart.setContent(emf.getHtmlBody(), "text/html");
            mpalt.addBodyPart(htmlpart);              

            
            // Add the .wav file as an attachment
            if(attachWav) {
                MimeBodyPart wavBodyPart = new MimeBodyPart();                 
            
                File file = m_vmessage.getAudioFile();  
               
                DataSource dataSource = new FileDataSource(file)  
                {  
                    public String getContentType()  
                    {  
                        return "audio/x-wav";  
                    }  
                };  
               
                wavBodyPart.setDataHandler(new DataHandler(dataSource));  
                wavBodyPart.setFileName(file.getName());  
                wavBodyPart.setHeader("Content-Transfer-Encoding", "base64");  
                wavBodyPart.setDisposition(Part.ATTACHMENT);  

                // Create a top level multipart/mixed part
                Multipart mpmixed = new MimeMultipart();
                // Make a new part to wrap the multipart/alternative part
                MimeBodyPart altpart = new MimeBodyPart();
                altpart.setContent(mpalt);
                
                // Add the alt part to the mixed part
                mpmixed.addBodyPart(altpart);
                // Add the wav part to the mixed part
                mpmixed.addBodyPart(wavBodyPart);
                // Use the mixed part as the content
                message.setContent(mpmixed);
            } else {
                // Use the alt part as the content
                message.setContent(mpalt);
            }
            //TODO message.setHeader("X-Priority", "1"); when we support priority
            return message;
        }
        
        /**
         * Build and send the message as e-mails to the recipients
         */
        public void run() {
            String to = m_mailbox.getMailboxPreferences().getEmailAddress();
            String alt = m_mailbox.getMailboxPreferences().getAlternateEmailAddress();
            
            LOG.debug("Emailer::run started");

            // Send to the main e-mail address
            if (to != null) {
                try {
                    LOG.info(String.format("Emailer::run sending message %s as e-mail to %s",
                            m_vmessage.getMessageId(), to));
                    boolean attachWav = m_mailbox.getMailboxPreferences().isAttachVoicemailToEmail();
                    MimeMessage message;
                    message = buildMessage(attachWav);
                    message.addRecipient(MimeMessage.RecipientType.TO, new InternetAddress(to));
                    Transport.send(message);
                } catch (Exception e) {
                    LOG.error("Emailer::run problem sending email.", e) ;
                    e.printStackTrace();
                }
            }
            
            // Send to the alternate e-mail address
            // (this could be a bit better: if both main and alternative have the same value for 
            //  isAttachVoicemailxxx, one could create the message once and send it with two recipients)
            if(alt != null) {
                try {
                    LOG.info(String.format("Emailer::run sending message %s as e-mail to %s",
                            m_vmessage.getMessageId(), alt));
                    boolean attachWav = m_mailbox.getMailboxPreferences().isAttachVoicemailToAlternateEmail();
                    MimeMessage message = buildMessage(attachWav);
                    message.addRecipient(MimeMessage.RecipientType.TO, new InternetAddress(alt));
                    Transport.send(message);
                } catch (Exception e) {
                    LOG.error("Emailer::run problem sending alternate email.", e) ;
                    e.printStackTrace();
                }
            }
            LOG.debug("Emailer::run finished");
        }
    }
    
    public static Emailer getEmailer() {
        if (s_me == null) {
            s_me = new Emailer();
        }
        return s_me;
    }
    
    static public void init(Configuration ivrConfig) {
        Emailer me = getEmailer();
        me.m_ivrConfig = ivrConfig;
        me.m_es = Executors.newCachedThreadPool();
        // Setup mail server
        Properties props = System.getProperties();
        props.put("mail.smtp.host", "localhost");
        props.put("mail.smtp.user", "postmaster"); // TODO get from ivrConfig
        me.m_session = Session.getDefaultInstance(props, null);  
    }

    public static boolean isJustTesting() {
        return getEmailer().m_justTesting;
    }

    public static void setJustTesting(boolean justTesting) {
        getEmailer().m_justTesting = justTesting;
    }
    
    public static void submit(BackgroundMailer bm) {
        getEmailer().m_es.submit(bm);
    }
}
