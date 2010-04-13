/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.ParseException;
import java.util.Date;
import java.util.List;

import com.thoughtworks.xstream.XStream;
import com.thoughtworks.xstream.annotations.XStreamAlias;
import com.thoughtworks.xstream.annotations.XStreamImplicit;
import com.thoughtworks.xstream.converters.basic.DateConverter;
import com.thoughtworks.xstream.io.xml.DomDriver;
import com.thoughtworks.xstream.mapper.MapperWrapper;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.XstreamFieldMapper;

/**
 * LEGEND INTO FILE DIRECTORY ========================================= For the 18th message in a
 * voicemail box
 *
 * 00000018-00.sta zero length file, exists only if message is unheard
 *
 * 00000018-00.wav if forwarded represent the comment, NOTE: can be zero if no comment was left
 * else voicemail message media
 *
 * 00000018-00.xml message or comment details
 *
 * 00000018-01.wav original message without comment NOTE: only exists for forwarded messages
 *
 * 00000018-01.xml original message details NOTE: only exists for forwarded messages
 *
 * 00000018-FW.wav original message plus comment NOTE: only exists for forwarded messages
 *
 */
public class Voicemail implements Comparable {
    private final String m_messageId;
    private MessageDescriptor m_descriptor;
    private MessageDescriptor m_forwardedDescriptor;
    private final File m_mailbox;
    private final File m_userDirectory;

    public Voicemail(File mailstoreDirectory, String userId, String folderId, String messageId) {
        m_userDirectory = new File(mailstoreDirectory, userId);
        m_mailbox = new File(m_userDirectory, folderId);
        m_messageId = messageId;
    }

    public String getFolderId() {
        return m_mailbox.getName();
    }

    public String getUserId() {
        return m_mailbox.getParentFile().getName();
    }

    public String getMessageId() {
        return m_messageId;
    }

    public boolean isHeard() {
        return !(new File(getMailboxDirectory(), getMessageId() + "-00.sta").exists());
    }

    public File getMailboxDirectory() {
        return m_mailbox;
    }

    public File getMediaFile() {
        File f = getForwardedMediaFile();
        if (!f.exists()) {
            f = getPotentialMediaOrCommentMediaFile();
        }
        return f;
    }

    public boolean hasForwardComment() {
        return isForwarded() && getPotentialMediaOrCommentMediaFile().length() > 0;
    }

    public File getForwardedMediaFileWithoutComment() {
        return new File(getMailboxDirectory(), getMessageId() + "-01.wav");
    }

    public boolean isForwarded() {
        return getForwardedMediaFile().exists();
    }

    public MessageDescriptor getDescriptor() {
        if (m_descriptor == null) {
            m_descriptor = readMessageDescriptor(getDescriptorFile());
        }

        return m_descriptor;
    }

    public MessageDescriptor getForwardedDescriptor() {
        if (m_forwardedDescriptor == null) {
            m_forwardedDescriptor = readMessageDescriptor(getForwardedDescriptorFile());
        }

        return m_forwardedDescriptor;
    }

    public String getSubject() {
        return getDescriptor().getSubject();
    }

    public void setSubject(String subject) {
        getDescriptor().setSubject(subject);
    }

    public int compareTo(Object o) {
        if (o == null || o instanceof Voicemail) {
            return -1;
        }
        return getMessageId().compareTo(((Voicemail) o).getMessageId());
    }

    public void save() {
        FileOutputStream out = null;
        try {
            out = new FileOutputStream(getDescriptorFile());
            writeMessageDescriptor(getDescriptor(), out);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(out);
        }
    }

    protected File[] getAllFiles() {
        File[] files = getMailboxDirectory().listFiles(new FileFilterByMessageId(getMessageId()));
        return files;
    }

    /**
     * File acts as either message file or comment file
     */
    protected File getPotentialMediaOrCommentMediaFile() {
        return new File(getMailboxDirectory(), getMessageId() + "-00.wav");
    }

    protected File getForwardedDescriptorFile() {
        return new File(getMailboxDirectory(), getMessageId() + "-01.xml");
    }

    protected File getForwardedMediaFile() {
        return new File(getMailboxDirectory(), getMessageId() + "-FW.wav");
    }

    protected File getDescriptorFile() {
        return new File(getMailboxDirectory(), getMessageId() + "-00.xml");
    }

    protected static class MessageDescriptorFormatException extends RuntimeException {
        MessageDescriptorFormatException(String message, ParseException cause) {
            super(message, cause);
        }
    }

    protected MessageDescriptor readMessageDescriptor(File file) {
        FileInputStream descriptorFile = null;
        try {
            descriptorFile = new FileInputStream(file);
            return readMessageDescriptor(descriptorFile);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(descriptorFile);
        }
    }

    protected static MessageDescriptor readMessageDescriptor(InputStream in) throws IOException {
        XStream xstream = getXmlSerializer();
        MessageDescriptor md = (MessageDescriptor) xstream.fromXML(in);
        return md;
    }

    /**
     * Element order is not preserved!!!
     */
    protected static void writeMessageDescriptor(MessageDescriptor md, OutputStream out) throws IOException {
        XStream xstream = getXmlSerializer();
        // See http://xstream.codehaus.org/faq.html#XML
        // Section "Why does XStream not write XML in UTF-8?"
        out.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n".getBytes());
        xstream.toXML(md, out);
    }

    protected static XStream getXmlSerializer() {
        XStream xstream = new XStream(new DomDriver()) {
            @Override
            protected MapperWrapper wrapMapper(MapperWrapper next) {
                return new XstreamFieldMapper(next);
            }
        };
        xstream.processAnnotations(MessageDescriptor.class);

        String[] acceptableTimeFormat = new String[] {
            MessageDescriptor.TIMESTAMP_FORMAT_NO_ZONE
        };
        // NOTE: xtream's dateformatter uses fixed ENGLISH Locale, which
        // turns out is ok because mediaserver writes out timestamp in a fixed
        // format independent of OS locale.
        xstream.registerConverter(new DateConverter(MessageDescriptor.TIMESTAMP_FORMAT, acceptableTimeFormat));

        return xstream;
    }

    @XStreamAlias("messagedescriptor")
    public class MessageDescriptor {
        static final String TIMESTAMP_FORMAT = "EEE, d-MMM-yyyy hh:mm:ss aaa z";
        // see XCF-1519
        static final String TIMESTAMP_FORMAT_NO_ZONE = "EEE, d-MMM-yyyy hh:mm:ss aaa";

        private Date m_timestamp;
        private int m_durationsecs;
        private String m_subject;
        private String m_from;
        private String m_priority;
        private String m_id;

        @XStreamImplicit(itemFieldName = "otherrecipient")
        private List<String> m_otherRecipients;

        public int getDurationsecs() {
            return m_durationsecs;
        }

        public int getDurationMillis() {
            return getDurationsecs() * 1000;
        }

        public List<String> getOtherRecipients() {
            return m_otherRecipients;
        }

        public String getFrom() {
            return m_from;
        }

        public String getFromBrief() {
            return SipUri.extractFullUser(getFrom().replace('+', ' '));
        }

        public String getId() {
            return m_id;
        }

        public String getPriority() {
            return m_priority;
        }

        public String getSubject() {
            return m_subject;
        }

        void setSubject(String subject) {
            m_subject = subject;
        }

        public Date getTimestamp() {
            return m_timestamp;
        }
    }

    protected static class FileFilterByMessageId implements FilenameFilter {
        private final String m_messageIdPrefix;

        FileFilterByMessageId(String messageId) {
            m_messageIdPrefix = messageId + "-";
        }

        public boolean accept(File dir, String name) {
            return name.startsWith(m_messageIdPrefix);
        }
    }
}
