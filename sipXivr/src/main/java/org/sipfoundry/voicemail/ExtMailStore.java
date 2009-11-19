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
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.GeneralSecurityException;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.Vector;
import javax.activation.DataHandler;
import javax.activation.DataSource;
import javax.activation.FileDataSource;
import javax.mail.AuthenticationFailedException;
import javax.mail.Folder;
import javax.mail.Message;
import javax.mail.MessageRemovedException;
import javax.mail.MessagingException;
import javax.mail.Multipart;
import javax.mail.Part;
import javax.mail.Session;
import javax.mail.Store;
import javax.mail.Transport;
import javax.mail.Flags.Flag;
import javax.mail.event.MessageChangedEvent;
import javax.mail.event.MessageChangedListener;
import javax.mail.event.MessageCountEvent;
import javax.mail.event.MessageCountListener;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeBodyPart;
import javax.mail.internet.MimeMessage;
import javax.mail.internet.MimeMultipart;
import javax.mail.search.HeaderTerm;
import javax.mail.search.SearchTerm;

import com.sun.mail.imap.IMAPFolder;
import com.sun.mail.imap.IMAPMessage;
import com.sun.mail.util.MailSSLSocketFactory;

import javax.security.auth.Subject;
import javax.security.auth.callback.Callback;
import javax.security.auth.callback.CallbackHandler;
import javax.security.auth.callback.NameCallback;
import javax.security.auth.callback.PasswordCallback;
import javax.security.auth.login.LoginContext;
import javax.security.auth.login.LoginException;

import org.apache.commons.codec.binary.Base64;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.ImapInfo;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.sipxivr.GreetingType;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.MailboxPreferences;
import org.sipfoundry.voicemail.Greeting;

/**
 *   ExtMailStore is responsible for synchronizing messages between each SipX Mailbox
 *   and its corresponding e-mail inbox (IMAP folder). eg. if a message is read via e-amil
 *   client, corresponding message in SipX mailbox is also marked read
 *   Sipx mailbox becomes a cache for the IMAP folder view of the messages
 *   An association between a SipX message and its corresponding IMAP folder msg is
 *   maintained via a X-SIPX-MSGID MIME header that is included in the message sent
 *   via SMTP  
 */
public class ExtMailStore {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    
    private static SyncThread m_syncThread;

    // key is mbxid, value is IMAPConnection
    private static Map<String, IMAPConnection> m_connectionMap = Collections
            .synchronizedMap(new HashMap<String, IMAPConnection>());

    private static Map<Folder, String> m_folderMap = Collections.synchronizedMap(new HashMap<Folder, String>());

    private static class IMAPConnection {
        public Session m_session;
        public User m_user;
        public Folder m_infolder;
        public Store m_store;
        public boolean m_synching;
        public IdleThread m_iThread;
        // maps IMAP sequence numbers to SipX Message Ids
        public Map<Integer, String> m_msgIdMap;
    }

    /**
     *  one IdleThread thread per IMAP connection
     *  does nothing more than perform the IDLE command
     *  within a loop until the folder gets closed
     */
    private static class IdleThread extends Thread {
        private IMAPFolder m_folderToCheck;
        private User m_user;

        public IdleThread(User user, Folder infolder) {
            this.m_folderToCheck = (IMAPFolder) infolder;
            this.m_user = user;
        }

        public void run() {
            boolean running = true;
            while (running) {
                try {
                    m_folderToCheck.idle();
                } catch (IllegalStateException e) {
                    // folder has been closed
                    running = false;
                    CloseConnection(m_user.getUserName(), true);
                } catch (MessagingException e) {
                    running = false;
                    CloseConnection(m_user.getUserName(), true);
                }
            }
        }
    }

    private static void LogError(String funcName, String mbx, String errMsg) {
        LOG.error(funcName + " failed for mbx " + mbx + " : " + errMsg);
    }
    
    
    /**
     * main thread within the ExtMailStore object.
     * creates/destroys IMAP connections as mailboxes are added/deleted 
     * runs at a low priority
     */
    private static class SyncThread extends Thread {

        static long m_LastCheckedTime;
        
        public SyncThread() {
        }

        private static void  UpdateConnections() {
            // now deal with user config that has changed: imap server, port#,
            // useTLS, email address, synchronize or not
            IMAPConnection conn;

            Collection<IMAPConnection> connections = m_connectionMap.values();
            
            for (Iterator<IMAPConnection> it = connections.iterator(); it.hasNext();) {
                conn = it.next();
                             
                Mailbox mbx = new Mailbox(conn.m_user);      
             
                if(mbx.getLastModified() > m_LastCheckedTime) {                   
                    CloseConnection(conn.m_user.getUserName(), false);
                    it.remove();
                    OpenConnection(conn.m_user);
                }    
            }
            
            Date now = new Date();
            m_LastCheckedTime = now.getTime();            
        }    
        
        public void run() {
            boolean running = true;
            Vector<User> users, currUsers;
            ValidUsersXML validUsersXML = null;
            Collection<IMAPConnection> connections;
            IMAPConnection conn;

            currUsers = null;
            Date now = new Date();
            m_LastCheckedTime = now.getTime();    
            
            while (running) {
                try {
                    validUsersXML = ValidUsersXML.update(LOG, true);
                } catch (Exception e1) {
                    System.exit(1); // If you can't trust validUsers, who can you trust?
                }

                users = ValidUsersXML.GetUsers();
                if (!users.equals(currUsers)) {

                    // check for deleted mailboxes
                    connections = m_connectionMap.values();
                    for (Iterator<IMAPConnection> it = connections.iterator(); it.hasNext();) {
                        conn = it.next();

                        User user = validUsersXML.getUser(conn.m_user.getUserName());
                        boolean deleteit;
                        if (user != null) {
                            deleteit = !user.hasVoicemail();
                        } else {
                            deleteit = true;
                        }

                        if (deleteit) {
                            CloseConnection(conn.m_user.getUserName(), false);
                            it.remove();
                        }
                    }

                    for (User u : users) {
                        if (!m_connectionMap.containsKey(u.getUserName()) && u.hasVoicemail()) {
                            OpenConnection(u);
                        }
                    }

                    currUsers = users;
                }
                
                UpdateConnections();                
                
                try {
                    sleep(5000);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    running = false;
                }
            }
        }
    }

    public static void Initialize() {
        m_syncThread = new SyncThread();
        m_syncThread.start();
        m_syncThread.setPriority(Thread.MIN_PRIORITY);
    }

    /*
     * returns the SipX message id from the header of the 
     * IMAP message. This header is included in the e-mail
     * forwarded via SMTP by SipX to the e-mail server
     */
    public static String GetMsgId(IMAPMessage msg) {

        String msgid = "";

        try {
            msgid = msg.getHeader("X-SIPX-MSGID", ";");
        } catch (MessageRemovedException e) {
            // message no longer in folder
            return null;
        } catch (MessagingException e) {
            LogError("GetMsgId", "", e.getMessage());
        }

        return msgid;
    }

    private static void SaveInFolder(Mailbox mbx, GreetingType type, 
                                     boolean isSpokenName, File recording) {
        
        IMAPConnection conn = Connection(mbx.getUser().getUserName());
        
        // if user is not using the IMAP synchronization feature or IMAP server 
        // is not reachable then conn will be null and there is nothing to do
        if(conn != null ) {
            Folder grtfolder = null;

            if (!conn.m_synching) {
                try{
                    grtfolder = conn.m_store.getFolder("VoiceMail Greetings");
                    OpenFolder(grtfolder);
                
                    if(isSpokenName) {
                        AddGreetingToFolder(conn.m_session, grtfolder, mbx, "spoken name", recording);
                    } else {
                        AddGreetingToFolder(conn.m_session, grtfolder, mbx, type.getId(), recording);
                    }
                } catch (MessagingException e) {
                    LogError("SaveInFolder", mbx.getUser().getUserName(), e.getMessage());                   
                }
                
                try {
                    grtfolder.close(true);
                } catch (MessagingException e) {
                    LogError("SaveInFolder", mbx.getUser().getUserName(), e.getMessage());
                }
            }
        }
    }
       
    public static void SaveGreetingInFolder(Mailbox mbx, GreetingType type, File recording) {
        SaveInFolder(mbx, type, false, recording);
    }   

    public static void SaveSpokenNameInFolder(Mailbox mbx, File recording) {
        SaveInFolder(mbx, GreetingType.NONE, true, recording);
    }    
        
    public static void FetchBody(String mbxId, String msgId, File BodyFile) {
        if (!BodyFile.exists()) {
            IMAPConnection conn = Connection(mbxId);

            if (conn != null) {
                FetchBodyFromFolder(conn.m_infolder, msgId, BodyFile);
            }
        }
    }

    public static void MarkSaved(String mbxId, String msgId) {

        IMAPConnection conn = Connection(mbxId);
        if (conn != null) {
            if (!conn.m_synching) {
                IMAPMessage msg = GetMsg(conn.m_infolder, msgId);

                if (msg != null) {
                    try {
                        msg.setFlag(Flag.SEEN, true);
                        msg.setFlag(Flag.DELETED, false);
                    } catch (MessagingException e) {
                        LogError("MarkSaved", mbxId, e.getMessage());
                    }
                }
            }
        }
    }

    public static void MarkNew(String mbxId, String msgId) {

        IMAPConnection conn = Connection(mbxId);
        if (conn != null) {
            if (!conn.m_synching) {
                IMAPMessage msg = GetMsg(conn.m_infolder, msgId);

                if (msg != null) {
                    try {
                        msg.setFlag(Flag.SEEN, false);
                        msg.setFlag(Flag.DELETED, false);
                    } catch (MessagingException e) {
                        LogError("MarkNew", mbxId, e.getMessage());
                    }
                }
            }
        }
    }

    public static void MarkDeleted(String mbxId, String msgId) {

        IMAPConnection conn = Connection(mbxId);
        if (conn != null) {
            if (!conn.m_synching) {
                IMAPMessage msg = GetMsg(conn.m_infolder, msgId);

                if (msg != null) {
                    try {
                        msg.setFlag(Flag.DELETED, true);
                        msg.setFlag(Flag.SEEN, true);
                        conn.m_infolder.expunge();

                        // the MsgIdMap now needs to be rebuilt
                        conn.m_msgIdMap.clear();
                        SearchTerm st = new HeaderTerm("X-SIPX-MSG", "yes");
                        Message[] themsgs = conn.m_infolder.search(st);

                        IMAPMessage amsg;
                        for (Message themsg : themsgs) {
                            amsg = (IMAPMessage) themsg;
                            conn.m_msgIdMap.put(amsg.getMessageNumber(), GetMsgId(amsg));
                        }

                    } catch (MessagingException e) {
                        LogError("MarkDeleted", mbxId, e.getMessage());
                    }
                }
            }
        }
    }

    private static void SynchronizeGreetings(IMAPConnection conn, Mailbox mbx) {
        // synchronize the three possible greetings and mailbox spoken name

        Folder grtfolder = null;

        try {
            grtfolder = conn.m_store.getFolder("VoiceMail Greetings");
            OpenFolder(grtfolder);
            File TempFile;
            boolean success;

            for (GreetingType greettype : GreetingType.values()) {
                // Create in the deleted directory, so if somehow things fail, they'll get removed
                // as they age out.
                TempFile = File.createTempFile("wav_", null, 
                        new File(mbx.getDeletedDirectory()));
                success = FetchBodyFromFolder(grtfolder, greettype.getId(), TempFile);
                if (success) {
                    Greeting greet = new Greeting(mbx);
                    greet.saveGreetingFile(greettype, TempFile);
                } else {
                    TempFile.delete();
                }
            }

            FetchBodyFromFolder(grtfolder, "Spoken Name", mbx.getRecordedNameFile());

        } catch (Exception e) {
            LogError("SynchronizeGreetings", mbx.getUser().getUserName(), e.getMessage());
        }

        if (grtfolder != null) {
            try {
                grtfolder.close(false);
            } catch (MessagingException e) {
                LogError("SychronizeGreetings", mbx.getUser().getUserName(), e.getMessage());
            }
        }
    }

    private static void SynchronizeMailbox(Mailbox mbx, Messages msgs) {
        // search folder for msgs that contain a SipX Msg Id
        // for each message update corresponding VmMessage object
        // store SipX msg id in a msgid[] array, sort the array
        // traverse SipX mailbox looking for messages not in the msgid[] array
        // permanently delete such messages

        IMAPConnection conn = Connection(mbx.getUser().getUserName());
        if (conn != null) {

            conn.m_synching = true;

            SynchronizeGreetings(conn, mbx);

            SearchTerm st = new HeaderTerm("X-SIPX-MSG",  "yes");
            
            try {
                Message[] themsgs = conn.m_infolder.search(st);
                String msgIds[] = new String[themsgs.length];

                IMAPMessage msg;
                for (int i = 0; i < themsgs.length; i++) {

                    msg = (IMAPMessage) themsgs[i];

                    msgIds[i] = GetMsgId(msg);

                    conn.m_msgIdMap.put(msg.getMessageNumber(), msgIds[i]);
                    SynchronizeMsg(mbx, msgs, msg, msgIds[i]);
                }
                java.util.Arrays.sort(msgIds);

                List<VmMessage> theList = msgs.getDeleted();
                for (VmMessage vmMsg : theList) {
                    if (java.util.Arrays.binarySearch(msgIds, vmMsg.getMessageId()) < 0) {
                        msgs.deleteMessage(vmMsg);
                    }
                }

                theList = msgs.getInbox();
                for (VmMessage vmMsg : theList) {
                    if (java.util.Arrays.binarySearch(msgIds, vmMsg.getMessageId()) < 0) {
                        msgs.deleteMessage(vmMsg);
                    }
                }

                theList = msgs.getSaved();
                for (VmMessage vmMsg : theList) {
                    if (java.util.Arrays.binarySearch(msgIds, vmMsg.getMessageId()) < 0) {
                        msgs.deleteMessage(vmMsg);
                    }
                }
            } catch (MessagingException e) {
                LogError("SychronizeMailbox", mbx.getUser().getUserName(), e.getMessage());
            }
            conn.m_synching = false;
        }
    }

    private static boolean FetchBodyFromFolder(Folder thefolder, String msgId, File BodyFile) {

        IMAPMessage msg = GetMsg(thefolder, msgId);

        if (msg != null) {
            try {
                Multipart mp = (Multipart) msg.getContent();

                byte buf[] = new byte[2048];
                int len;
                Part part;
                String ConType;

                for (int i = 0, n = mp.getCount(); i < n; i++) {
                    part = mp.getBodyPart(i);
                    ConType = part.getContentType().toLowerCase();

                    if (ConType.startsWith("audio/x-wav")) {
                        OutputStream OutStream = new FileOutputStream(BodyFile);

                        InputStream InStream = part.getInputStream();
                        while ((len = InStream.read(buf)) > 0) {
                            OutStream.write(buf, 0, len);
                        }
                        OutStream.close();
                        break;
                    }
                }              
                
            } catch (Exception e) {
                LogError("FetchBodyFromFolder", "", e.getMessage());
            }
        }
        return msg != null;
    }

    /*
     * return the IMAP Mesasge from the inbox folder corresponding to the
     * SipX message Id. Does this by searching the folder for the message
     * whose X-SIPX-MSGID header value equals the msgId parameter 
     */
    
    private static IMAPMessage GetMsg(Folder flder, String msgId) {

        Message[] msgs;
        SearchTerm st = new HeaderTerm("X-SIPX-MSGID", msgId);
        try {
            msgs = flder.search(st);

            if(msgs.length > 0) {
                return((IMAPMessage)msgs[0]);
            }
          
        } catch (MessagingException e) {
            LogError("GetMsg", "", e.getMessage()); 
        }
        return null;
    }

    private static void AddGreetingToFolder(Session session, Folder folder, Mailbox mbx, String greetid, File voicefile) {
        MimeMessage message = new MimeMessage(session);
        try {
            // delete the old greeting
            IMAPMessage msg = GetMsg(folder, greetid);

            if (msg != null) {
                msg.setFlag(Flag.DELETED, true);
                folder.expunge();
            }

            message.addHeader("X-SIPX-MSGID", greetid);
            message.setSubject(greetid + " greeting");

            String ident = mbx.getUser().getIdentity();
            String from = "postmaster" + ident.substring(ident.indexOf('@'));
            message.setFrom(new InternetAddress(from, "Voicemail"));

            message.addRecipient(MimeMessage.RecipientType.TO, new InternetAddress(mbx.getUser()
                    .getEmailAddress()));

            Multipart mp = new MimeMultipart();
            MimeBodyPart wavBodyPart = new MimeBodyPart();

            DataSource dataSource = new FileDataSource(voicefile) {
                public String getContentType() {
                    {
                        return "audio/x-wav";
                    }
                };
            };

            wavBodyPart.setDataHandler(new DataHandler(dataSource));
            wavBodyPart.setFileName(voicefile.getName());
            wavBodyPart.setHeader("Content-Transfer-Encoding", "base64");
            wavBodyPart.setDisposition(Part.ATTACHMENT);
            mp.addBodyPart(wavBodyPart);
            message.setContent(mp);

            message.setFlag(Flag.SEEN, true);

            MimeMessage[] msgs = new MimeMessage[1];
            msgs[0] = message;

            folder.appendMessages(msgs);

        } catch (Exception e) {
            LogError("AddGreetingToFolder", mbx.getUser().getUserName(), e.getMessage());
        }
    }
  
    private static IMAPConnection Connection(String mbxid) {
        return m_connectionMap.get(mbxid);
    }

    private static String GetMbxId(IMAPMessage msg) {

        String mbxId = "";
        try {
            mbxId = msg.getHeader("X-SIPX-MBXID", ";");
        } catch (MessageRemovedException e) {
            // message no longer in folder
            return null;
        } catch (MessagingException e) {
            LogError("GeetMbxId", "", e.getMessage());
        }
        return mbxId;
    }

    private static void SendUpdatePswdEmail(IMAPConnection conn) {
        // send e-mail to username account at host      
        
        Mailbox mbox = new Mailbox(conn.m_user);        
        
        File statusfile = new File(mbox.getUserDirectory(), "email.sta");
        if (statusfile.exists()) {
            if (statusfile.lastModified() >= mbox.getLastModified()) {
                // don't send email only one time until administrator or user changes configuration
                return;
            }
        }                           
        
        try {           
            MimeMessage message = new MimeMessage(conn.m_session);

            String ident = conn.m_user.getIdentity();
            String from = "postmaster" + ident.substring(ident.indexOf('@'));
            message.setFrom(new InternetAddress(from, "Voicemail"));

            message.addRecipient(MimeMessage.RecipientType.TO, new InternetAddress(conn.m_user.getEmailAddress()));

            message.setSubject("e-mail password change required");
            message.setText("You must change your e-mail password by logging in at " + "http://"
                    + ident.substring(ident.indexOf('@') + 1));
            Transport.send(message);
            
            // sent email now create or re-create stats file
            if (statusfile.exists()) {
                statusfile.delete();
            }
            statusfile.createNewFile();
            
        } catch (Exception e) {
            LogError("SendUpdatePswdEmail", conn.m_user.getUserName(), e.getMessage());
        }
    }

    private static void OpenFolder(Folder folder) {

        try {
            if (!folder.exists()) {
                folder.create(Folder.HOLDS_MESSAGES);
            }
            folder.open(Folder.READ_WRITE);

        } catch (MessagingException e) {
            LogError("OpenFolder", "", e.getMessage());
        }
    }

    private static void OpenConnection(final User user) {

        final MessageChangedListener changelistener = new MessageChangedListener() {
            public void messageChanged(MessageChangedEvent ev) {
                IMAPMessage msg = (IMAPMessage) ev.getMessage();
                try {
                    if (msg.isSet(Flag.SEEN)) {
                        String mbxid = GetMbxId(msg);
                        if (mbxid != null) {
                            // one of our messages
                            IMAPConnection conn = Connection(mbxid);

                            String msgid = GetMsgId(msg);
                            Mailbox mbx = new Mailbox(conn.m_user);
                            Messages msgs = Messages.newMessages(mbx);

                            conn.m_synching = true;
                            VmMessage VmMsg = getVmMessage(msgs, msgid);
                            msgs.markMessageHeard(VmMsg, true);
                            conn.m_synching = false;
                        }                     
                    }
                } catch (MessagingException e) {
                    LogError("messageChanged", "", e.getMessage());
                }                       
            }
        };

        final MessageCountListener countListener = new MessageCountListener() {

            public void messagesAdded(MessageCountEvent e) {
                // If msg forwarded to inbox via SMTP we should
                // still sync the message as it may have been read or deleted on
                // SipX
                Message[] somemsgs = e.getMessages();

                String mbxid = null;
                Messages mymsgs = null;
                IMAPConnection conn = null;
                Mailbox mbx = null;
                String msgid;

                for(Message aMsg : somemsgs) {
                    msgid = GetMsgId((IMAPMessage) aMsg);
                    if (msgid != null) {
                        if (mbxid == null) {
                            mbxid = GetMbxId((IMAPMessage)aMsg);
                            conn = Connection(mbxid);
                            mbx = new Mailbox(conn.m_user);
                            conn.m_synching = true;
                            mymsgs = Messages.newMessages(mbx);
                        }
                        conn.m_msgIdMap.put(aMsg.getMessageNumber(), msgid);

                        SynchronizeMsg(mbx, mymsgs, (IMAPMessage)aMsg, msgid);
                    }
                }
                if (conn != null) {
                    conn.m_synching = false;
                }
            }

            public void messagesRemoved(MessageCountEvent e) {
                if (e.isRemoved()) {
                    return;
                }
                
                String mbxid;
                String msgid;
                Message removedMsg = e.getMessages()[0];
                
                /**
                 * unfortunately JavaMail doesn't allow one to query the msg headers
                 * for an expunged message even if it has previously cached the headers so
                 * need to use the m_folderMap to map to mailbox id of the corresponding
                 * SipX mailbox
                 */
                mbxid = m_folderMap.get(e.getSource());
                IMAPConnection conn = Connection(mbxid);

                msgid = conn.m_msgIdMap.get(removedMsg.getMessageNumber());

                if (msgid != null) {
                    Mailbox mbx = new Mailbox(conn.m_user);
                    conn.m_synching = true;
                    Messages msgs = Messages.newMessages(mbx);
                    VmMessage VmMsg = getVmMessage(msgs, msgid);
                    msgs.deleteMessage(VmMsg);
                    conn.m_msgIdMap.remove(removedMsg.getMessageNumber());
                }

                conn.m_synching = false;
            }
        };
        
        final IMAPConnection conn = new IMAPConnection();
        conn.m_synching = false;
        final String mbxid = user.getUserName();
        
        // should we even try to make a connection? 
        ImapInfo imapInfo = user.getImapInfo();
        if(imapInfo == null || !imapInfo.isSynchronize())
            return;
                
        class ConnectAction implements java.security.PrivilegedAction {
            public Object run() {
               
                try {     
                    conn.m_user = user;
                    ImapInfo imapInfo = user.getImapInfo();
 
                    conn.m_store.connect(imapInfo.getHost(), Integer.parseInt(imapInfo.getPort()), 
                                         imapInfo.getAccount(), imapInfo.getDecodedPassword());
                    
                    conn.m_infolder = conn.m_store.getFolder("inbox");
                    OpenFolder(conn.m_infolder);
                    
                    conn.m_msgIdMap = Collections.synchronizedMap(new HashMap<Integer, String>());

                    m_connectionMap.put(mbxid, conn);
                    m_folderMap.put(conn.m_infolder, mbxid);

                    conn.m_infolder.addMessageChangedListener(changelistener);
                    conn.m_infolder.addMessageCountListener(countListener);
                    conn.m_iThread = new IdleThread(user, conn.m_infolder);
                    conn.m_iThread.start();

                    Mailbox mbx = new Mailbox(user);
                    Mailbox.createDirsIfNeeded(mbx);
                    Messages themsgs = Messages.newMessages(mbx);
                    SynchronizeMailbox(mbx, themsgs);

                } catch (AuthenticationFailedException e) {
                    SendUpdatePswdEmail(conn);
                } catch (MessagingException e) {
                    // e.printStackTrace();
                    SendUpdatePswdEmail(conn); 
                }
                return null;
            }
        }

        try {
            Properties props = new Properties();
            props.setProperty("mail.imap.sasl.enable", "true");
            props.setProperty("mail.imap.starttls.enable", "true");
            
            if(imapInfo.isUseTLS()) {
                props.setProperty("mail.imap.ssl.enable", "true");
                
                MailSSLSocketFactory sf = new MailSSLSocketFactory();
                sf.setTrustAllHosts(true);
                props.put("mail.imap.ssl.socketFactory", sf);                               
                
            } else {
                props.setProperty("mail.imap.ssl.enable", "false");
            }
                
            props.setProperty("mail.smtp.host", "localhost");

            conn.m_session = Session.getInstance(props);           
                        
            conn.m_store = conn.m_session.getStore("imap");
            // to deal with GSSAPI security we need to create our own
            // LoginContext. Then try to make the connection
            // using Subject.doAs with the authenticated 
            // subject associated with the context

            CallbackHandler cbh = new CallbackHandler() {
                public void handle(Callback[] callbacks) {
                    for (int i = 0; i < callbacks.length; i++) {
                        if (callbacks[i] instanceof NameCallback) {
                            NameCallback ncb = (NameCallback) callbacks[i];
                            ncb.setName(conn.m_user.getImapInfo().getAccount());
                        } else if (callbacks[i] instanceof PasswordCallback) {
                            PasswordCallback pcb = (PasswordCallback) callbacks[i];
                            pcb.setPassword(conn.m_user.getImapInfo().getDecodedPassword().toCharArray());
                        }
                    }
                }
            };
            
            LoginContext lc = new LoginContext("", new Subject(), cbh);
            lc.login();
            Subject sub = lc.getSubject();

            if (sub != null) {
                Subject.doAs(sub, new ConnectAction());
            }

        } catch (LoginException e) {
            // creating a login context failed
            // try plain password style login instead
            Subject.doAs(null, new ConnectAction());
        } catch (SecurityException e) {
            // creating a login context failed
            // try plain password style login instead
            Subject.doAs(null, new ConnectAction());
        } catch (MessagingException e) {
            // connection likely lost            
            CloseConnection(mbxid, true);
            
        } catch (GeneralSecurityException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    private static VmMessage getVmMessage(Messages msgs, String msgid) {
        VmMessage VmMsg;

        VmMsg = msgs.m_inbox.get(msgid);
        if (VmMsg == null) {
            VmMsg = msgs.m_saved.get(msgid);
            if (VmMsg == null) {
                VmMsg = msgs.m_deleted.get(msgid);
            }
        }
        return VmMsg;
    }

    private static synchronized void  CloseConnection(String mbx, boolean removeFromMap) {
        IMAPConnection conn = Connection(mbx);

        if (conn != null) {
            m_folderMap.remove(conn.m_infolder);
            if (removeFromMap) {
                m_connectionMap.remove(mbx);
            }
            try {
                conn.m_infolder.close(false);
                conn.m_store.close();
            } catch (MessagingException e) {
                // nothing to do
            }
        }
    }
    
    /*
     *  e-mail client may have
     *   - marked the msg read
     *   - marked the msg deleted
     *   - marked the msg new
     *  need to update VmMessage view  
     */ 
    private static void SynchronizeMsg(Mailbox mbx, Messages msgs, IMAPMessage msg, String msgId) {

        VmMessage vmMsg;
        boolean alreadyMarkedDeleted = false;
        boolean alreadySaved = false;

        vmMsg = msgs.m_inbox.get(msgId);
        if (vmMsg == null) {
            vmMsg = msgs.m_saved.get(msgId);
            if (vmMsg == null) {
                vmMsg = msgs.m_deleted.get(msgId);
                alreadyMarkedDeleted = vmMsg != null;
            } else {
                alreadySaved = true;
            }
        }

        if (vmMsg != null) {
            try {
                if (msg.isSet(Flag.DELETED) & !alreadyMarkedDeleted) {
                    msgs.deleteMessage(vmMsg);
                } else {
                    if (msg.isSet(Flag.SEEN) & !alreadySaved) {
                        msgs.saveMessage(vmMsg);
                    }
                }

            } catch (MessagingException e) {
                LogError("SychronizeMsg", mbx.getUser().getUserName(), e.getMessage());
            }
        } else {
            // message not found at all .. create msg in mailbox with correct status
            // and same msgid as what the e-mail server has
            // don't down load voice body .. only do that if the user subsequently
            // tries to play the message
            vmMsg = org.sipfoundry.voicemail.VmMessage.newMessageFromMime(mbx, msg);
        }
    }
}
