/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */

package org.sipfoundry.faxrx;

import java.io.File;
import java.io.FileOutputStream;
import java.net.URL;
import java.util.Locale;
import java.util.Properties;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.activation.DataHandler;
import javax.activation.DataSource;
import javax.activation.FileDataSource;
import javax.activation.MimetypesFileTypeMap;
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
import org.sipfoundry.commons.util.RemoteRequest;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

import com.lowagie.text.Document;
import com.lowagie.text.Image;
import com.lowagie.text.PageSize;
import com.lowagie.text.Rectangle;
import com.lowagie.text.pdf.PdfContentByte;
import com.lowagie.text.pdf.PdfWriter;
import com.lowagie.text.pdf.RandomAccessFileOrArray;
import com.lowagie.text.pdf.codec.TiffImage;

public class FaxProcessor implements ApplicationContextAware {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private ExecutorService m_es;
    private Session m_session;
    private ApplicationContext m_appContext;
    private String m_faxFormat = "pdf";
    private String m_sendIMUrl;

    public void init() {
        m_es = Executors.newCachedThreadPool();
        // Setup mail server
        Properties props = System.getProperties();
        props.put("mail.smtp.host", "localhost");
        props.put("mail.smtp.user", "postmaster");
        m_session = Session.getDefaultInstance(props, null);
    }

    /**
     * Queue up sending the VmMessage as an e-mail to the addresses specified in the mailbox
     *
     * @param mailbox
     * @param vmessage
     */
    public void queueFaxProcessing(User user, File faxFile, String remoteStationId, String channelId,
            String channelIdNumber, int faxPages, boolean rxSuccess, String resultCode, String resultText) {
        BackgroundFaxProcessor processor = new BackgroundFaxProcessor(user, faxFile, remoteStationId, channelId,
                channelIdNumber, faxPages, rxSuccess, resultCode, resultText);
        m_es.submit(processor);
    }

    /**
     * The Runnable class that builds and sends the e-mail
     */
    class BackgroundFaxProcessor implements Runnable {
        private User m_user;
        private File m_faxFile;
        private String m_remoteStationId;
        private String m_channelIdName;
        private String m_channelIdNumber;
        private int m_faxPages;
        private boolean m_success;
        private String m_resultCode;
        private String m_resultText;

        BackgroundFaxProcessor(User user, File faxFile, String remoteStationId, String channelIdName,
                String channelIdNumber, int faxPages, boolean rxSuccess, String resultCode, String resultText) {
            m_user = user;
            m_faxFile = faxFile;
            m_remoteStationId = remoteStationId;
            m_channelIdName = channelIdName;
            m_channelIdNumber = channelIdNumber;
            m_faxPages = faxPages;
            m_success = rxSuccess;
            m_resultCode = resultCode;
            m_resultText = resultText;
        }

        /**
         * Build and send the message as e-mails to the recipients
         */
        public void run() {
            File emailAttachment = null;
            // define attachment format
            String faxInfo;
            if (m_faxFormat.equals("pdf")) {
                // convert TIFF to PDF
                File converted = tiff2Pdf(m_faxFile);
                if (converted != null) {
                    // if conversion is succesful attach the PDF file
                    emailAttachment = converted;
                    m_faxFile.delete();
                } else {
                    // if conversion is not succesful attach the TIFF file and alert
                    emailAttachment = m_faxFile;
                    LOG.error("Fax Receive: Could not convert TIFF to PDF. TIFF will be attached");
                }
            } else {
                emailAttachment = m_faxFile;
            }

            // construct a reasonable faxInfo string to be used as part of the email
            // subject and instant message. Send email even if faxReceive.rxSuccess() false
            // as there could be incomplete faxes - act as a fax machine
            String name = null;
            String number = null;

            if (m_remoteStationId != null) {
                name = m_remoteStationId;
            } else {
                if (!m_channelIdName.equals("unknown")) {
                    name = m_channelIdName;
                }
            }

            if (!m_channelIdNumber.equals("0000000000")) {
                number = m_channelIdNumber;
            }

            Locale userLocale = m_user.getLocale();
            faxInfo = m_faxPages + " " + m_appContext.getMessage("page_fax_from", null, "page fax from", userLocale)
                    + " ";
            if (name != null) {
                faxInfo += name + " ";
            }

            if (number != null) {
                faxInfo += "(" + number + ")";
            }

            if (name == null && number == null) {
                faxInfo += m_appContext.getMessage("an_unknown_sender", null, "an unknown sender", userLocale);
            }

            // need to send to at least one email address
            boolean sent = false;
            String faxSubject = m_appContext.getMessage("Your", null, "Your", userLocale) + " " + faxInfo;

            if (m_user.getEmailFormat() != EmailFormats.FORMAT_NONE) {
                sendEmail(m_user.getEmailAddress(), emailAttachment, faxSubject);
                sent = true;
            }

            if (m_user.getAltEmailFormat() != EmailFormats.FORMAT_NONE) {
                sendEmail(m_user.getAltEmailAddress(), emailAttachment, faxSubject);
                sent = true;
            }

            // need to send to at least one email address so let's be more aggressive

            if (!sent) {
                if (m_user.getEmailAddress() != null) {
                    sendEmail(m_user.getEmailAddress(), emailAttachment, faxSubject);
                    sent = true;
                }
            }

            if (!sent) {
                // need to send to at least one email address so let's be even more aggressive
                if (m_user.getAltEmailAddress() != null) {
                    sendEmail(m_user.getAltEmailAddress(), emailAttachment, faxSubject);
                } else {
                    // didn't send anywhere !!
                    LOG.error("Fax Receive: No email address for user " + m_user.getUserName());
                }
            }

            if (m_success) {
                LOG.debug("Fax received successfully " + faxInfo);
                sendIM(m_user, m_appContext.getMessage("You_received_a", null, "You received a", userLocale) + " "
                        + faxInfo + ".");
            } else {
                LOG.error("Fax receive failed from " + m_channelIdNumber + ". Error text: " + m_resultText
                        + ". Error code: " + m_resultCode);
                sendIM(m_user,
                        m_appContext.getMessage("You_received_an_incomplete", null, "You received an incomplete",
                                userLocale) + " " + faxInfo + ".");
            }

            emailAttachment.delete();
        }
    }

    private void sendIM(User user, String instantMsg) {
        URL sendIMUrl;

        if (m_sendIMUrl == null) {
            return;
        }

        try {
            sendIMUrl = new URL(m_sendIMUrl + "/" + user.getUserName() + "/sendFaxReceiveIM");

            RemoteRequest rr = new RemoteRequest(sendIMUrl, "text/plain", instantMsg);
            if (!rr.http()) {
                LOG.error("faxrx::sendIM Trouble with RemoteRequest " + rr.getResponse());
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private File tiff2Pdf(File tiffFile) {
        Pattern pattern = Pattern.compile("(.*).tiff");
        Matcher matcher = pattern.matcher(tiffFile.getName());
        boolean matchFound = matcher.find();

        // check if tiffFile is actually a TIFF file, just in case
        if (matchFound) {
            // located at default tmp-file directory
            File pdfFile = new File(System.getProperty("java.io.tmpdir"), matcher.group(1) + ".pdf");
            try {
                // read TIFF file
                RandomAccessFileOrArray tiff = new RandomAccessFileOrArray(tiffFile.getAbsolutePath());

                // get number of pages of TIFF file
                int pages = TiffImage.getNumberOfPages(tiff);

                // create PDF file
                Document pdf = new Document(PageSize.LETTER, 0, 0, 0, 0);

                PdfWriter writer = PdfWriter.getInstance(pdf, new FileOutputStream(pdfFile));
                writer.setStrictImageSequence(true);

                // open PDF filex
                pdf.open();

                PdfContentByte contentByte = writer.getDirectContent();

                // write PDF file page by page
                for (int page = 1; page <= pages; page++) {
                    Image temp = TiffImage.getTiffImage(tiff, page);
                    temp.scalePercent(7200f / temp.getDpiX(), 7200f / temp.getDpiY());
                    pdf.setPageSize(new Rectangle(temp.getScaledWidth(), temp.getScaledHeight()));
                    temp.setAbsolutePosition(0, 0);
                    contentByte.addImage(temp);
                    pdf.newPage();
                }
                // close PDF file
                pdf.close();
            } catch (Exception e) {
                LOG.error("faxrx::tiff2Pdf error " + e.getMessage());
                e.printStackTrace();
                return null;
            }
            return pdfFile;
        }

        else {
            return null;
        }
    }

    private void sendEmail(String emailAddr, final File emailAttachment, String faxSubject) {

        if (emailAddr == null) {
            return;
        }

        MimeMessage message = new MimeMessage(m_session);

        try {
            message.addRecipient(MimeMessage.RecipientType.TO, new InternetAddress(emailAddr));

            String senderName = m_appContext.getMessage("SenderName", null, null);
            String senderMailTo = m_appContext.getMessage("SenderMailto", null, null);
            message.setFrom(new InternetAddress(String.format("%s <%s>", senderName, senderMailTo)));

            message.setSubject(faxSubject, "UTF-8");

            MimeBodyPart faxBodyPart = new MimeBodyPart();

            DataSource dataSource = new FileDataSource(emailAttachment) {
                public String getContentType() {
                    MimetypesFileTypeMap mimeTypes = new MimetypesFileTypeMap();
                    mimeTypes.addMimeTypes("image/tiff tiff TIFF");
                    mimeTypes.addMimeTypes("application/pdf pdf PDF");
                    return mimeTypes.getContentType(emailAttachment);
                }
            };

            faxBodyPart.setDataHandler(new DataHandler(dataSource));
            faxBodyPart.setFileName(emailAttachment.getName());
            faxBodyPart.setHeader("Content-Transfer-Encoding", "base64");
            faxBodyPart.setDisposition(Part.ATTACHMENT);

            Multipart mpmixed = new MimeMultipart();
            mpmixed.addBodyPart(faxBodyPart);
            message.setContent(mpmixed);

            Transport.send(message);

        } catch (AddressException e) {
            LOG.error("faxrx::sendEmail error " + e.getMessage());
        } catch (MessagingException e) {
            LOG.error("faxrx::sendEmail error " + e.getMessage());
        }
    }

    @Override
    public void setApplicationContext(ApplicationContext context) {
        m_appContext = context;
    }

    public void setSendImUrl(String url) {
        m_sendIMUrl = url;
    }

    public void setFaxFormat(String faxFormat) {
        m_faxFormat = faxFormat;
    }

}
