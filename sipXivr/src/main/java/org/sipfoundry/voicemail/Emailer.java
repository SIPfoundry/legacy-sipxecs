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
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.User.EmailFormats;
import org.sipfoundry.sipxivr.IvrConfiguration;
import org.sipfoundry.sipxivr.Mailbox;

public class Emailer {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private static Emailer s_me;
    boolean m_justTesting;
    ExecutorService m_es;
    Session m_session;
    IvrConfiguration m_ivrConfig;

    private Emailer() {}
    
    /**
     * Queue up sending the VmMessage as an e-mail to the addresses specified in the mailbox
     * 
     * @param mailbox
     * @param vmessage
     */
    public static void queueVm2Email(Mailbox mailbox, VmMessage vmessage) {
        if (isJustTesting()) {
            return;
        }

        User u = mailbox.getUser();
        if (u.getEmailFormat() != EmailFormats.FORMAT_NONE ||
            u.getAltEmailFormat() != EmailFormats.FORMAT_NONE) {
            LOG.info("Emailer::queueVm2Email queuing e-mail for "+u.getIdentity());
            BackgroundMailer bm = getEmailer().new BackgroundMailer(mailbox, vmessage);
            submit(bm);
        }
    }

    /**
     * The Runnable class that builds and sends the e-mail
     */
    class BackgroundMailer implements Runnable {
        VmMessage m_vmessage;
        Mailbox m_mailbox;

        BackgroundMailer(Mailbox mailbox, VmMessage vmessage) {
            m_vmessage = vmessage;
            m_mailbox = mailbox;
        }
        
        /**
         * Build up the MIME multipart formatted e-mail
         * 
         * @param attachAudio Attach the audio file as part of the message
         * @return
         * @throws AddressException
         * @throws MessagingException
         */
        javax.mail.Message buildMessage(EmailFormatter emf, boolean attachAudio) throws AddressException, MessagingException {
            MimeMessage message = new MimeMessage(m_session);
            message.setFrom(new InternetAddress(emf.getSender()));  

            message.setSubject(emf.getSubject(), "UTF-8");
            
            message.addHeader("X-SIPX-MSGID", m_vmessage.getMessageId());
            message.addHeader("X-SIPX-MBXID", m_mailbox.getUser().getUserName());
            message.addHeader("X-SIPX-MSG", "yes");
            
            String htmlBody = emf.getHtmlBody();
            String textBody = emf.getTextBody();
            
            // Use multipart/alternative if we are attaching audio or there is an html body part
            if (attachAudio || htmlBody != null && htmlBody.length() > 0) {
                // Create an "mulipart/alternative" part with text and html alternatives
                Multipart mpalt = new MimeMultipart("alternative");
    
                if (textBody != null && textBody.length() > 0) {
                    // Add the text part of the message first
                    MimeBodyPart textPart = new MimeBodyPart();
                    textPart.setText(textBody, "UTF-8"); // UTF-8 in case there's Unicode in there
                    mpalt.addBodyPart(textPart);
                }
    
                if (htmlBody != null && htmlBody.length() > 0) {
                    // Add the HTML part of the message
                    MimeBodyPart htmlpart = new MimeBodyPart();
                    htmlpart.setContent(htmlBody, "text/html");
                    mpalt.addBodyPart(htmlpart);              
                }
    
                // Add the audio file as an attachment
                if(attachAudio) {
                    MimeBodyPart audioBodyPart = new MimeBodyPart();                 
                
                    File file = m_vmessage.getAudioFile();  
                   
                    DataSource dataSource = new FileDataSource(file)  
                    {  
                        public String getContentType()  
                        {  
                            return "audio/x-wav";  
                        }  
                    };  
                   
                    audioBodyPart.setDataHandler(new DataHandler(dataSource));  
                    audioBodyPart.setFileName(file.getName());  
                    audioBodyPart.setHeader("Content-Transfer-Encoding", "base64");  
                    audioBodyPart.setDisposition(Part.ATTACHMENT);  
    
                    // Create a top level multipart/mixed part
                    Multipart mpmixed = new MimeMultipart();
                    
                    // Make a new part to wrap the multipart/alternative part
                    if(mpalt.getCount() > 0) {
                        MimeBodyPart altpart = new MimeBodyPart();
                        altpart.setContent(mpalt);
                        // Add the alt part to the mixed part
                        mpmixed.addBodyPart(altpart);
                    }                 
                    
                    // Add the wav part to the mixed part
                    mpmixed.addBodyPart(audioBodyPart);
                    // Use the mixed part as the content
                    message.setContent(mpmixed);
                } else {
                    // Use the alt part as the content
                    message.setContent(mpalt); // JavaMail guesses content type
                }
            } else {
                if (textBody != null && textBody.length() > 0) {
                    // Just a text part, use a simple message
                    message.setText(textBody, "UTF-8");
                }
            }
            
            //TODO message.setHeader("X-Priority", "1"); when we support priority
            return message;
        }
        
        /**
         * Build and send the message as e-mails to the recipients
         */
        public void run() {
            User u = m_mailbox.getUser();
            String to = u.getEmailAddress();
            String alt = u.getAltEmailAddress();
            
            LOG.debug("Emailer::run started");

            EmailFormats fmt = u.getEmailFormat();
            // Send to the main e-mail address
            if (fmt != EmailFormats.FORMAT_NONE) {
                try {
                    boolean attachAudio = u.isAttachAudioToEmail();
                    LOG.info(String.format("Emailer::run sending message %s as %s email to %s %s audio",
                            m_vmessage.getMessageId(), fmt.toString(), to, attachAudio?"with":"without"));
                    EmailFormatter emf = EmailFormatter.getEmailFormatter(fmt, 
                            m_ivrConfig, m_mailbox, m_vmessage);
                    javax.mail.Message message = buildMessage(emf, attachAudio);
                    message.addRecipient(MimeMessage.RecipientType.TO, new InternetAddress(to));
                    Transport.send(message);
                } catch (Exception e) {
                    LOG.error("Emailer::run problem sending email.", e) ;
                }
            }
            
            // Send to the alternate e-mail address
            // (this could be a bit better: if both main and alternative have the same value for 
            //  fmt and isAttachVoicemailxxx, one could create the message once and send it with two recipients)
            fmt = u.getAltEmailFormat();
            if(fmt != EmailFormats.FORMAT_NONE) {
                try {
                    boolean attachAudio = u.isAltAttachAudioToEmail();
                    LOG.info(String.format("Emailer::run sending message %s as %s email to %s %s audio",
                            m_vmessage.getMessageId(), fmt.toString(), to, attachAudio?"with":"without"));
                    EmailFormatter emf = EmailFormatter.getEmailFormatter(fmt, 
                            m_ivrConfig, m_mailbox, m_vmessage);
                    javax.mail.Message message = buildMessage(emf, attachAudio);

                    message.addRecipient(MimeMessage.RecipientType.TO, new InternetAddress(alt));
                    Transport.send(message);
                } catch (Exception e) {
                    LOG.error("Emailer::run problem sending alternate email.", e) ;
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
    
    static public void init(IvrConfiguration ivrConfig) {
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
