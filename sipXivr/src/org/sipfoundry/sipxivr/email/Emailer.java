/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxivr.email;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
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
import javax.mail.util.ByteArrayDataSource;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.User.EmailFormats;
import org.sipfoundry.voicemail.mailbox.Folder;
import org.sipfoundry.voicemail.mailbox.VmMessage;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

public class Emailer implements ApplicationContextAware {
    private static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private ExecutorService m_es;
    private Session m_session;
    private ApplicationContext m_context;
    private String m_audioFormat;

    public void init() {
        m_es = Executors.newCachedThreadPool();
        // Setup mail server
        Properties props = System.getProperties();
        props.put("mail.smtp.host", "localhost");
        props.put("mail.smtp.user", "postmaster"); // TODO get from ivrConfig
        m_session = Session.getDefaultInstance(props, null);
    }

    /**
     * Queue up sending the VmMessage as an e-mail to the addresses specified in the mailbox
     *
     * @param mailbox
     * @param vmessage
     */
    public void queueVm2Email(User destUser, VmMessage vmessage) {
        if (destUser.getEmailFormat() != EmailFormats.FORMAT_NONE
                || destUser.getAltEmailFormat() != EmailFormats.FORMAT_NONE) {
            if (vmessage.getParentFolder().equals(Folder.CONFERENCE)) {
                LOG.info("Emailer::do not queue email for conferences");
                return;
            }
            LOG.info("Emailer::queueVm2Email queuing e-mail for " + destUser.getIdentity());
            BackgroundMailer bm = new BackgroundMailer(destUser, vmessage);
            submit(bm);
        }
    }

    /**
     * The Runnable class that builds and sends the e-mail
     */
    class BackgroundMailer implements Runnable {
        VmMessage m_vmessage;
        User m_user;

        BackgroundMailer(User destUser, VmMessage vmessage) {
            m_vmessage = vmessage;
            m_user = destUser;
        }

        /**
         * Build up the MIME multipart formatted e-mail
         *
         * @param attachAudio Attach the audio file as part of the message
         * @return
         * @throws AddressException
         * @throws MessagingException
         */
        javax.mail.Message buildMessage(EmailFormatter emf, boolean attachAudio) throws AddressException,
                MessagingException, IOException {
            MimeMessage message = new MimeMessage(m_session);
            message.setFrom(new InternetAddress(emf.getSender()));

            message.setSubject(emf.getSubject(), "UTF-8");

            message.addHeader("X-SIPX-FROMURI", m_vmessage.getDescriptor().getFromUri());
            message.addHeader("X-SIPX-MSGID", m_vmessage.getMessageId());
            message.addHeader("X-SIPX-MBXID", m_user.getUserName());
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
                    // Add the IMAGEs part of the message
                    insertMimeImage(mpalt, "images/play_50x50.png", "imageListen");
                    insertMimeImage(mpalt, "images/inbox_50x50.png", "imageInbox");
                    insertMimeImage(mpalt, "images/delete_50x50.png", "imageDelete");
                }

                // Add the audio file as an attachment
                if (attachAudio) {
                    MimeBodyPart audioBodyPart = new MimeBodyPart();

                    File file = m_vmessage.getAudioFile();

                    DataSource dataSource = new FileDataSource(file) {
                        @Override
                        public String getContentType() {
                            return getMimeType();
                        }
                    };

                    audioBodyPart.setDataHandler(new DataHandler(dataSource));
                    audioBodyPart.setFileName(file.getName());
                    audioBodyPart.setHeader("Content-Transfer-Encoding", "base64");
                    audioBodyPart.setDisposition(Part.ATTACHMENT);

                    // Create a top level multipart/mixed part
                    Multipart mpmixed = new MimeMultipart();

                    // Make a new part to wrap the multipart/alternative part
                    if (mpalt.getCount() > 0) {
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

            if (m_vmessage.isUrgent()) {
                message.setHeader("X-Priority", "1");
            }
            return message;
        }

        private String getMimeType() {
            if (m_audioFormat.equals("mp3")) {
                return "audio/x-mp3";
            }
            return "audio/x-wav";
        }

        /**
         *
         * @param mpalt - multipart instance previously created that contains all HTML text
         *        including image keys sample: <img src="cid:[imageKey]">
         * @param imageSource - image source file name embedded in *this* class jar file
         * @param imageKey - the image key value as it is written in the multipart html content
         * @throws MessagingException
         * @throws IOException
         */
        private void insertMimeImage(Multipart mpalt, String imageSource, String imageKey)
                throws MessagingException, IOException {
            MimeBodyPart imagePart = new MimeBodyPart();
            InputStream playImage = Emailer.class.getClassLoader().getResourceAsStream(imageSource);
            DataSource fds = new ByteArrayDataSource(playImage, "image/png");
            imagePart.setDataHandler(new DataHandler(fds));
            StringBuilder builder = new StringBuilder();
            builder.append("<").append(imageKey).append(">");
            imagePart.setHeader("Content-ID", builder.toString());
            mpalt.addBodyPart(imagePart);
        }

        /**
         * Build and send the message as e-mails to the recipients
         */
        @Override
        public void run() {
            String to = m_user.getEmailAddress();
            String alt = m_user.getAltEmailAddress();

            LOG.debug("Emailer::run started");

            EmailFormats fmt = m_user.getEmailFormat();
            // Send to the main e-mail address
            if (fmt != EmailFormats.FORMAT_NONE) {
                try {
                    boolean attachAudio = m_user.isAttachAudioToEmail();
                    LOG.info(String.format("Emailer::run sending message %s as %s email to %s %s audio",
                            m_vmessage.getMessageId(), fmt.toString(), to, attachAudio ? "with" : "without"));
                    EmailFormatter emf = getEmailFormatter(fmt, m_user, m_vmessage);
                    javax.mail.Message message = buildMessage(emf, attachAudio);
                    message.addRecipient(MimeMessage.RecipientType.TO, new InternetAddress(to));
                    Transport.send(message);
                } catch (Exception e) {
                    LOG.error("Emailer::run problem sending email.", e);
                }
            }

            // Send to the alternate e-mail address
            // (this could be a bit better: if both main and alternative have the same value for
            // fmt and isAttachVoicemailxxx, one could create the message once and send it with
            // two recipients)
            fmt = m_user.getAltEmailFormat();
            if (fmt != EmailFormats.FORMAT_NONE) {
                try {
                    boolean attachAudio = m_user.isAltAttachAudioToEmail();
                    LOG.info(String.format("Emailer::run sending message %s as %s email to %s %s audio",
                            m_vmessage.getMessageId(), fmt.toString(), alt, attachAudio ? "with" : "without"));
                    EmailFormatter emf = getEmailFormatter(fmt, m_user, m_vmessage);
                    javax.mail.Message message = buildMessage(emf, attachAudio);

                    message.addRecipient(MimeMessage.RecipientType.TO, new InternetAddress(alt));
                    Transport.send(message);
                } catch (Exception e) {
                    LOG.error("Emailer::run problem sending alternate email.", e);
                }
            }
            m_vmessage.cleanup();
            LOG.debug("Emailer::run finished");
        }
    }

    /**
     * Factory method to return the right EmailFormatter based on emailFormat
     * @param emailFormat
     * @param ivrConfig
     * @param mailbox
     * @param vmessage
     * @return
     */
    private EmailFormatter getEmailFormatter(EmailFormats emailFormat, User user, VmMessage vmessage) {
        EmailFormatter formatter = m_context.getBean(emailFormat.getId(), EmailFormatter.class);
        formatter.init(user, vmessage);
        return formatter;
    }

    public void submit(BackgroundMailer bm) {
        m_es.submit(bm);
    }

    @Override
    public void setApplicationContext(ApplicationContext context) {
        m_context = context;
    }

    public void setAudioFormat(String format) {
        m_audioFormat = format;
    }
}
