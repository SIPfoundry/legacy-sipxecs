/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.commons.userdb;

import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Vector;

import org.sipfoundry.commons.userdb.User.EmailFormats;

import com.mongodb.BasicDBList;
import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;
import com.mongodb.Mongo;
import com.mongodb.QueryBuilder;

/**
 * Holds the valid user data needed for the AutoAttendant, parsing from mongo db imdb
 * 
 */
public enum ValidUsers {
    INSTANCE;

    // Mapping of letters to DTMF numbers.
    // Position of letter in letters maps to corresponding position in numbers
    private static String LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    private static String NUMBERS = "22233344455566677778889999";
    private static final String IMDB_USERBUSYPROMPT = "bsyprmpt";
    private static final String IMDB_VOICEMAILTUI = "vcmltui";
    private static final String IMDB_EMAIL = "email";
    private static final String IMDB_NOTIFICATION = "notif";
    private static final String IMDB_ATTACH_AUDIO = "attaudio";
    private static final String IMDB_ALT_EMAIL = "altemail";
    private static final String IMDB_ALT_NOTIFICATION = "altnotif";
    private static final String IMDB_ALT_ATTACH_AUDIO = "altattaudio";
    private static final String IMDB_SYNC = "synch";
    private static final String IMDB_HOST = "host";
    private static final String IMDB_PORT = "port";
    private static final String IMDB_TLS = "tls";
    private static final String IMDB_ACCOUNT = "acnt";
    private static final String IMDB_PASSWD = "pswd";
    private static final String IMDB_DISPLAY_NAME = "dspl";
    private static final String IMDB_HASHED_PASSTOKEN = "hshpstk";
    private static final String IMDB_ID = "id";
    private static final String IMDB_UID = "uid";
    private static final String IMDB_VALID = "vld";
    private static final String IMDB_ALIASSES = "als";
    private static final String IMDB_ALIAS = "alias";
    private static final String IMDB_PERMISSION = "prm";
    private static final String IMDB_CONTACT = "cnt";
    private static final String IMDB_IDENTITY = "ident";
    private static final String IMDB_PERM_AA = "AutoAttendant";
    private static final String IMDB_PERM_VOICEMAIL = "Voicemail";
    private static final String IMDB_PERM_RECPROMPTS = "RecordSystemPrompts";
    private static final String IMDB_PERM_TUICHANGEPIN = "tui-change-pin";
    private static final String IMDB_RELATION = "rln";
    private static final String IMDB_PINTOKEN = "pntk";

    private static String HOST = "localhost";
    private static int PORT = 27017;
    private Mongo m_mongoInstance;

    public List<User> getUsers() {
        List<User> users = new ArrayList<User>();
        try {
            DBCursor cursor = getEntityCollection().find(QueryBuilder.start(IMDB_VALID).is(Boolean.TRUE).get());
            Iterator<DBObject> objects = cursor.iterator();
            while (objects.hasNext()) {
                DBObject validUser = objects.next();
                if (!validUser.get(IMDB_ID).toString().startsWith("User")) {
                    BasicDBList aliasesObj = (BasicDBList) validUser.get(IMDB_ALIASSES);
                    if (aliasesObj != null) {
                        for (int i = 0; i < aliasesObj.size(); i++) {
                            DBObject aliasObj = (DBObject) aliasesObj.get(i);
                            users.add(extractValidUserFromAlias(aliasObj));
                        }
                    }
                } else {
                    users.add(extractValidUser(validUser));
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return users;
    }

    /**
     * See if a given user_name is valid (aka it can be dialed and reach a user)
     * 
     * @param userNname
     * 
     * @return user found or null
     */
    public User getUser(String userName) {
        DBObject queryUserName = QueryBuilder.start(IMDB_VALID).is(Boolean.TRUE).and(IMDB_UID).is(userName).get();
        DBObject result = getEntityCollection().findOne(queryUserName);
        if (result != null) {
            return extractValidUser(getEntityCollection().findOne(queryUserName));
        }

        // check aliases
        BasicDBObject elemMatch = new BasicDBObject();
        elemMatch.put(IMDB_ID, userName);
        BasicDBObject alias = new BasicDBObject();
        alias.put("$elemMatch", elemMatch);
        BasicDBObject queryAls = new BasicDBObject();
        queryAls.put(IMDB_ALIASSES, alias);
        queryAls.put(IMDB_VALID, Boolean.TRUE);
        DBObject aliasResult = getEntityCollection().findOne(queryAls);
        if (aliasResult == null) {
            return null;
        }
        if (!aliasResult.get(IMDB_ID).toString().startsWith("User")) {
            BasicDBList aliases = (BasicDBList) aliasResult.get(IMDB_ALIASSES);
            for (int i = 0; i < aliases.size(); i++) {
                DBObject aliasObj = (DBObject) aliases.get(i);
                if (getStringValue(aliasObj, IMDB_ID).equals(userName)) {
                    return extractValidUserFromAlias(aliasObj);
                }
            }
        }
        return extractValidUser(aliasResult);
    }

    /**
     * Given a bunch of DTMF digits, return the list of users that matches
     * 
     * @param digits DTMF digits to match against user directory
     * @param onlyVoicemailUsers limit match to users in directory who have voicemail
     * @return a Vector of users that match
     */
    public List<User> lookupDTMF(String digits, boolean onlyVoicemailUsers) {
        List<User> matches = new ArrayList<User>();
        BasicDBList permList = new BasicDBList();
        permList.add(IMDB_PERM_AA);
        BasicDBObject inDirectory = new BasicDBObject();
        inDirectory.put("$all", permList);
        BasicDBObject hasDisplayName = new BasicDBObject();
        hasDisplayName.put("$exists", Boolean.TRUE);
        BasicDBObject queryAls = new BasicDBObject();
        queryAls.put(IMDB_PERMISSION, inDirectory);
        queryAls.put(IMDB_VALID, Boolean.TRUE);
        queryAls.put(IMDB_DISPLAY_NAME, hasDisplayName);
        DBCursor aliasResult = getEntityCollection().find(queryAls);
        Iterator<DBObject> objects = aliasResult.iterator();
        while (objects.hasNext()) {
            User user = extractValidUser(objects.next());
            if (user.getDialPatterns() != null) {
                for (String dialPattern : user.getDialPatterns()) {
                    if (dialPattern.startsWith(digits)) {
                        if (!onlyVoicemailUsers || user.hasVoicemail()) {
                            matches.add(user);
                            break;
                        }
                    }
                }
            }
        }
        return matches;
    }

    private DBCollection getEntityCollection() {
        DBCollection entity = null;
        try {
            if (m_mongoInstance == null) {
                m_mongoInstance = new Mongo(HOST, PORT);
            }
            DB imdb = m_mongoInstance.getDB("imdb");
            entity = imdb.getCollection("entity");
        } catch (UnknownHostException ex) {
            System.exit(1);
        }
        return entity;
    }

    private static User extractValidUserFromAlias(DBObject aliasObj) {
        if (aliasObj == null) {
            return null;
        }
        User user = new User();
        String id = getStringValue(aliasObj, IMDB_ID);
        user.setIdentity(id);
        user.setUserName(id);
        user.setUri(getStringValue(aliasObj, IMDB_CONTACT));
        user.setInDirectory(false);
        return user;
    }

    private static User extractValidUser(DBObject obj) {
        if (obj == null) {
            return null;
        }
        if (!Boolean.valueOf(obj.get(IMDB_VALID).toString())) {
            return null;
        }

        User user = new User();
        user.setIdentity(getStringValue(obj, IMDB_IDENTITY));
        user.setUserName(getStringValue(obj, IMDB_UID));
        user.setDisplayName(getStringValue(obj, IMDB_DISPLAY_NAME));
        user.setUri(getStringValue(obj, IMDB_CONTACT));
        user.setPasstoken(getStringValue(obj, IMDB_HASHED_PASSTOKEN));
        user.setPintoken(getStringValue(obj, IMDB_PINTOKEN));

        BasicDBList permissions = (BasicDBList) obj.get(IMDB_PERMISSION);
        if (permissions != null) {
            user.setInDirectory(permissions.contains(IMDB_PERM_AA));
            user.setHasVoicemail(permissions.contains(IMDB_PERM_VOICEMAIL));
            user.setCanRecordPrompts(permissions.contains(IMDB_PERM_RECPROMPTS));
            user.setCanTuiChangePin(permissions.contains(IMDB_PERM_TUICHANGEPIN));
        }

        user.setUserBusyPrompt(Boolean.valueOf(getStringValue(obj, IMDB_USERBUSYPROMPT)));
        user.setVoicemailTui(getStringValue(obj, IMDB_VOICEMAILTUI));
        user.setEmailAddress(getStringValue(obj, IMDB_EMAIL));
        if (obj.keySet().contains(IMDB_NOTIFICATION)) {
            user.setEmailFormat(getStringValue(obj, IMDB_NOTIFICATION));
        }
        user.setAttachAudioToEmail(Boolean.valueOf(getStringValue(obj, IMDB_ATTACH_AUDIO)));

        user.setAltEmailAddress(getStringValue(obj, IMDB_ALT_EMAIL));
        if (obj.keySet().contains(IMDB_ALT_NOTIFICATION)) {
            user.setAltEmailFormat(getStringValue(obj, IMDB_ALT_NOTIFICATION));
        }
        user.setAltAttachAudioToEmail(Boolean.valueOf(getStringValue(obj, IMDB_ALT_ATTACH_AUDIO)));

        BasicDBList aliasesObj = (BasicDBList) obj.get(IMDB_ALIASSES);
        if (aliasesObj != null) {
            Vector<String> aliases = new Vector<String>();
            for (int i = 0; i < aliasesObj.size(); i++) {
                DBObject aliasObj = (DBObject) aliasesObj.get(i);
                if (aliasObj.get(IMDB_RELATION).toString().equals(IMDB_ALIAS)) {
                    aliases.add(aliasObj.get(IMDB_ID).toString());
                }
            }
            user.setAliases(aliases);
        }

        if (obj.keySet().contains(IMDB_SYNC)) {
            ImapInfo imapInfo = new ImapInfo();
            imapInfo.setSynchronize(Boolean.valueOf(getStringValue(obj, IMDB_SYNC)));
            imapInfo.setHost(getStringValue(obj, IMDB_HOST));
            imapInfo.setPort(getStringValue(obj, IMDB_PORT));
            imapInfo.setUseTLS(Boolean.valueOf(getStringValue(obj, IMDB_TLS)));
            imapInfo.setAccount(getStringValue(obj, IMDB_ACCOUNT));
            imapInfo.setPassword(getStringValue(obj, IMDB_PASSWD));
            user.setImapInfo(imapInfo);
            if (imapInfo.isSynchronize()) {
                user.setEmailFormat(EmailFormats.FORMAT_IMAP);
                user.setAttachAudioToEmail(true);
            }
            // // If account isn't set, use the e-mail username
            if (imapInfo.getAccount() == null || imapInfo.getAccount().length() == 0) {
                if (user.getEmailAddress() != null) {
                    imapInfo.setAccount(user.getEmailAddress().split("@")[0]);
                }
            }
        }

        if (user.isInDirectory()) {
            buildDialPatterns(user);
        }

        return user;
    }

    private static String getStringValue(DBObject obj, String key) {
        if (obj.keySet().contains(key)) {
            if (obj.get(key) != null) {
                return obj.get(key).toString();
            }
        }
        return null;
    }

    /**
     * Remove all non-letter characters, convert to upper case Remove diacritical marks if
     * possible
     * 
     * @param orig
     */
    private static String compress(String orig) {
        if (orig == null) {
            return "";
        }

        String normal = orig.toUpperCase();

        // Brute force conversion of diacriticals
        normal = normal.replaceAll("[ÂÀÄÁÃ]", "A");
        normal = normal.replaceAll("[ÊÈËÉ]", "E");
        normal = normal.replaceAll("[ÎÌÏÍ]", "I");
        normal = normal.replaceAll("[ÔÒÖÓÕ]", "O");
        normal = normal.replaceAll("[ÛÙÜÚ]", "U");
        normal = normal.replaceAll("Ç", "C");
        normal = normal.replaceAll("Ñ", "N");

        // Remove non letters
        normal = normal.replaceAll("[^A-Z]", "");

        return normal;
    }

    /**
     * Map from letters to DTMF numbers
     */
    private static String mapDTMF(String orig) {
        if (orig == null) {
            return "";
        }

        StringBuilder output = new StringBuilder(orig.length());
        for (int i = 0; i < orig.length(); i++) {
            String c = orig.substring(i, i + 1);
            int pos = LETTERS.indexOf(c);
            if (pos >= 0) {
                // Output the corresponding position in numbers
                output.append(NUMBERS.charAt(pos));
            }
        }
        return output.toString();
    }

    public static String getDomainPart(String uri) {
        if (uri == null) {
            return null;
        }

        String domainName = uri;
        int atStart = domainName.indexOf("@");
        if (atStart <= 0) {
            return null;
        }

        domainName = domainName.substring(atStart + 1);
        return domainName.trim();
    }

    public static String getUserPart(String uri) {
        if (uri == null) {
            return null;
        }

        String userName = uri;
        int urlStart = userName.indexOf("sip:");

        if (urlStart >= 0) {
            userName = userName.substring(urlStart + 4);
        } else {
            int gtStart = userName.indexOf("<");
            if (gtStart >= 0) {
                userName = userName.substring(gtStart + 1);
            }
        }

        int atStart = userName.indexOf("@");
        if (atStart <= 0) {
            return null;
        }

        userName = userName.substring(0, atStart);
        return userName.trim();
    }

    public static String getDisplayPart(String uri) {
        if (uri == null) {
            return null;
        }

        String displayName = "";
        int urlStart = uri.indexOf("sip:");

        if (urlStart < 0) {
            return null;
        }

        displayName = uri.substring(0, urlStart);
        displayName = displayName.trim();
        int gtStart = displayName.indexOf("<");
        if (gtStart >= 0) {
            displayName = displayName.substring(0, gtStart);
        }
        int quoteStart = displayName.indexOf('"');
        if (quoteStart >= 0) {
            displayName = displayName.substring(quoteStart + 1);
            int quoteEnd = displayName.indexOf('"');
            if (quoteEnd >= 0) {
                displayName = displayName.substring(0, quoteEnd);
            }
        }
        return displayName.trim();
    }

    /**
     * Parse the Display name into a list of DTMF sequences
     * 
     * Do one for Last name first And one for First name first
     * 
     * @param u
     */
    private static void buildDialPatterns(User u) {
        u.setDialPatterns(new Vector<String>());

        if (u.getDisplayName() == null) {
            return;
        }

        String[] names = u.getDisplayName().split("\\W");
        // Remove all non-character data, convert to upper case, convert to DTMF

        LinkedList<String> queue = new LinkedList<String>();
        for (String name : names) {
            String dtmf = mapDTMF(compress(name));
            if (dtmf.length() > 0) {
                queue.add(mapDTMF(compress(name)));
            }
        }

        // Given a b c d, generate:
        // a b c d
        // b c d a
        // c d a b
        // d a b c
        for (int i = 0; i < queue.size(); i++) {
            String mashup;
            String first = queue.poll(); // Pull first
            mashup = first;
            for (int j = 0; j < queue.size(); j++) {
                String next = queue.poll();
                mashup += next;
                queue.add(next);
            }
            queue.add(first); // Put first back (its now last)
            u.getDialPatterns().add(mashup);
        }
    }
}
