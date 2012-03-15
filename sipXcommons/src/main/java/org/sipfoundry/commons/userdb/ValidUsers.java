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
package org.sipfoundry.commons.userdb;

import static org.sipfoundry.commons.mongo.MongoConstants.ACCOUNT;
import static org.sipfoundry.commons.mongo.MongoConstants.ACTIVEGREETING;
import static org.sipfoundry.commons.mongo.MongoConstants.ALIAS;
import static org.sipfoundry.commons.mongo.MongoConstants.ALIASES;
import static org.sipfoundry.commons.mongo.MongoConstants.ALIAS_ID;
import static org.sipfoundry.commons.mongo.MongoConstants.ALT_ATTACH_AUDIO;
import static org.sipfoundry.commons.mongo.MongoConstants.ALT_EMAIL;
import static org.sipfoundry.commons.mongo.MongoConstants.ALT_IM_ID;
import static org.sipfoundry.commons.mongo.MongoConstants.ALT_NOTIFICATION;
import static org.sipfoundry.commons.mongo.MongoConstants.ATTACH_AUDIO;
import static org.sipfoundry.commons.mongo.MongoConstants.AVATAR;
import static org.sipfoundry.commons.mongo.MongoConstants.BUTTONS;
import static org.sipfoundry.commons.mongo.MongoConstants.CELL_PHONE_NUMBER;
import static org.sipfoundry.commons.mongo.MongoConstants.COMPANY_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_ENTRY_IM;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_EXIT_IM;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_EXT;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_OWNER;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_PIN;
import static org.sipfoundry.commons.mongo.MongoConstants.CONTACT;
import static org.sipfoundry.commons.mongo.MongoConstants.DIALPAD;
import static org.sipfoundry.commons.mongo.MongoConstants.DISPLAY_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.DISTRIB_LISTS;
import static org.sipfoundry.commons.mongo.MongoConstants.EMAIL;
import static org.sipfoundry.commons.mongo.MongoConstants.FAX_NUMBER;
import static org.sipfoundry.commons.mongo.MongoConstants.GROUPS;
import static org.sipfoundry.commons.mongo.MongoConstants.HASHED_PASSTOKEN;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_PHONE_NUMBER;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_CITY;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_COUNTRY;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_STATE;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_STREET;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_ZIP;
import static org.sipfoundry.commons.mongo.MongoConstants.HOST;
import static org.sipfoundry.commons.mongo.MongoConstants.ID;
import static org.sipfoundry.commons.mongo.MongoConstants.IDENTITY;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ENABLED;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ID;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ON_THE_PHONE_MESSAGE;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_PASSWORD;
import static org.sipfoundry.commons.mongo.MongoConstants.JOB_DEPT;
import static org.sipfoundry.commons.mongo.MongoConstants.JOB_TITLE;
import static org.sipfoundry.commons.mongo.MongoConstants.ITEM;
import static org.sipfoundry.commons.mongo.MongoConstants.LANGUAGE;
import static org.sipfoundry.commons.mongo.MongoConstants.LEAVE_MESSAGE_BEGIN_IM;
import static org.sipfoundry.commons.mongo.MongoConstants.LEAVE_MESSAGE_END_IM;
import static org.sipfoundry.commons.mongo.MongoConstants.NOTIFICATION;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_CITY;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_COUNTRY;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_STATE;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_STREET;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_ZIP;
import static org.sipfoundry.commons.mongo.MongoConstants.OPERATOR;
import static org.sipfoundry.commons.mongo.MongoConstants.PASSWD;
import static org.sipfoundry.commons.mongo.MongoConstants.PERMISSIONS;
import static org.sipfoundry.commons.mongo.MongoConstants.PERSONAL_ATT;
import static org.sipfoundry.commons.mongo.MongoConstants.PINTOKEN;
import static org.sipfoundry.commons.mongo.MongoConstants.PORT;
import static org.sipfoundry.commons.mongo.MongoConstants.RELATION;
import static org.sipfoundry.commons.mongo.MongoConstants.SYNC;
import static org.sipfoundry.commons.mongo.MongoConstants.TLS;
import static org.sipfoundry.commons.mongo.MongoConstants.UID;
import static org.sipfoundry.commons.mongo.MongoConstants.USERBUSYPROMPT;
import static org.sipfoundry.commons.mongo.MongoConstants.USER_LOCATION;
import static org.sipfoundry.commons.mongo.MongoConstants.VALID_USER;
import static org.sipfoundry.commons.mongo.MongoConstants.VOICEMAILTUI;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Vector;
import java.util.regex.Pattern;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.commons.userdb.User.EmailFormats;

import com.mongodb.BasicDBList;
import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;
import com.mongodb.QueryBuilder;

/**
 * Holds the valid user data needed for the AutoAttendant, parsing from mongo db imdb
 * 
 */
public class ValidUsers {
    // Mapping of letters to DTMF numbers.
    // Position of letter in letters maps to corresponding position in numbers
    private static String LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    private static String NUMBERS = "22233344455566677778889999";
    private static final String IMDB_PERM_AA = "AutoAttendant";
    private static final String IMDB_PERM_VOICEMAIL = "Voicemail";
    private static final String IMDB_PERM_RECPROMPTS = "RecordSystemPrompts";
    private static final String IMDB_PERM_TUICHANGEPIN = "tui-change-pin";
    private DB m_imdb;

    /**
     * Loading all users into memory is an extremely expensive call for large systems (10K-50K user system).
     * Consider refactoring your code to not call this method.
     * 
     * @return
     */
    public List<User> getValidUsers() {
        List<User> users = new ArrayList<User>();
        try {
            DBCursor cursor = getEntityCollection().find(QueryBuilder.start(VALID_USER).is(Boolean.TRUE).get());
            Iterator<DBObject> objects = cursor.iterator();
            while (objects.hasNext()) {
                DBObject validUser = objects.next();
                if (!validUser.get(ID).toString().startsWith("User")) {
                    BasicDBList aliasesObj = (BasicDBList) validUser.get(ALIASES);
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
     * Loading all users into memory is an extremely expensive call for large systems (10K-50K user system).
     * Consider refactoring your code to not call this method.
     * 
     * @return
     */
    public DBCursor getUsers() {
        Pattern userPattern = Pattern.compile("User.*");
        BasicDBObject query = new BasicDBObject(ID, userPattern);
        DBCursor cursor = getEntityCollection().find(query);
        return cursor;
    }

    /**
     * Loading all users into memory is an extremely expensive call for large systems (10K-50K user system).
     * Consider refactoring your code to not call this method.
     * 
     * @return
     */
    public List<User> getUsersWithImEnabled() {
        List<User> users = new ArrayList<User>();
        DBCursor cursor = getEntityCollection().find(QueryBuilder.start(IM_ENABLED).is(Boolean.TRUE).get());
        Iterator<DBObject> objects = cursor.iterator();
        while (objects.hasNext()) {
            DBObject user = objects.next();
            users.add(extractUser(user));
        }
        return users;
    }

    /**
     * Returns a list of all im ids (of users with im enabled)
     * 
     * @return
     */
    public List<String> getAllImIdsInGroup(String group) {
        List<String> imIds = new ArrayList<String>();
        DBCursor cursor = getEntityCollection().find(
                QueryBuilder.start(GROUPS).is(group).and(IM_ENABLED).is(Boolean.TRUE).get());
        Iterator<DBObject> objects = cursor.iterator();
        while (objects.hasNext()) {
            DBObject user = objects.next();
            imIds.add(user.get(IM_ID).toString());
        }
        return imIds;
    }

    /**
     * See if a given user_name is valid (aka it can be dialed and reach a user)
     * 
     * @param userNname
     * 
     * @return user found or null
     */
    public User getUser(String userName) {
        if (userName == null) {
            return null;
        }
        DBObject queryUserName = QueryBuilder.start(VALID_USER).is(Boolean.TRUE).and(UID).is(userName).get();
        DBObject result = getEntityCollection().findOne(queryUserName);
        if (result != null) {
            return extractValidUser(result);
        }

        // check aliases
        BasicDBObject elemMatch = new BasicDBObject();
        elemMatch.put(ALIAS_ID, userName);
        BasicDBObject alias = new BasicDBObject();
        alias.put("$elemMatch", elemMatch);
        BasicDBObject queryAls = new BasicDBObject();
        queryAls.put(ALIASES, alias);
        queryAls.put(VALID_USER, Boolean.TRUE);
        DBObject aliasResult = getEntityCollection().findOne(queryAls);
        if (aliasResult == null) {
            return null;
        }
        if (!aliasResult.get(ID).toString().startsWith("User")) {
            BasicDBList aliases = (BasicDBList) aliasResult.get(ALIASES);
            for (int i = 0; i < aliases.size(); i++) {
                DBObject aliasObj = (DBObject) aliases.get(i);
                if (getStringValue(aliasObj, ALIAS_ID).equals(userName)) {
                    return extractValidUserFromAlias(aliasObj);
                }
            }
        }
        return extractValidUser(aliasResult);
    }

    public User getUserByConferenceName(String conferenceName) {
        DBObject queryConference = QueryBuilder.start(CONF_NAME).is(conferenceName).get();
        DBObject conferenceResult = getEntityCollection().findOne(queryConference);
        if (conferenceResult != null && getStringValue(conferenceResult, CONF_OWNER) != null) {
            User user = getUser(getStringValue(conferenceResult, CONF_OWNER));
            addConference(user, conferenceResult);
            return user;
        }
        return null;
    }

    public User getUserByJid(String jid) {
        return getUserByJidObject(jid);
    }

    public User getUserByInsensitiveJid(String jid) {
        Pattern insensitiveJid = Pattern.compile(jid, Pattern.CASE_INSENSITIVE);
        return getUserByJidObject(insensitiveJid);
    }

    private User getUserByJidObject(Object jid) {
        BasicDBObject jidQuery = new BasicDBObject();
        jidQuery.put(IM_ID, jid);
        BasicDBObject altJidQuery = new BasicDBObject();
        altJidQuery.put(ALT_IM_ID, jid);
        DBObject queryJid = QueryBuilder.start().or(jidQuery, altJidQuery).get();
        DBObject jidResult = getEntityCollection().findOne(queryJid);
        User user = extractValidUser(jidResult);
        if (user != null) {
            DBObject queryConference = QueryBuilder.start(CONF_OWNER).is(user.getUserName()).get();
            DBObject conferenceResult = getEntityCollection().findOne(queryConference);
            if (conferenceResult != null) {
                addConference(user, conferenceResult);
            }
        }
        return user;
    }

    public List<User> getUsersUpdatedAfter(Long ms) {
        List<User> users = new ArrayList<User>();
        Pattern userPattern = Pattern.compile("User.*");
        DBObject query = QueryBuilder.start(ID).is(userPattern).and(MongoConstants.TIMESTAMP).greaterThan(ms).get();
        DBCursor cursor = getEntityCollection().find(query);
        Iterator<DBObject> objects = cursor.iterator();
        while (objects.hasNext()) {
            DBObject user = objects.next();
            users.add(extractUser(user));
        }
        return users;
    }
    
    public DBCursor getEntitiesWithPermissions() {
        DBObject query = QueryBuilder.start(PERMISSIONS).exists(true).get();
        DBCursor cursor = getEntityCollection().find(query);
        return cursor;
    }

    public DBCursor getEntitiesWithPermission(String name) {
        DBObject query = QueryBuilder.start(PERMISSIONS).exists(true).and(PERMISSIONS).is(name).get();
        DBCursor cursor = getEntityCollection().find(query);
        return cursor;
    }

    public DBCursor getUsersInBranch(String name) {
        DBObject query = QueryBuilder.start(USER_LOCATION).is(name).get();
        DBCursor cursor = getEntityCollection().find(query);
        return cursor;
    }

    public DBCursor getUsersInGroup(String name) {
        DBObject query = QueryBuilder.start(GROUPS).is(name).get();
        DBCursor cursor = getEntityCollection().find(query);
        return cursor;
    }

    private void addConference(User user, DBObject conference) {
        user.setConfName(getStringValue(conference, CONF_NAME));
        user.setConfNum(getStringValue(conference, CONF_EXT));
        user.setConfPin(getStringValue(conference, CONF_PIN));
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
        queryAls.put(PERMISSIONS, inDirectory);
        queryAls.put(VALID_USER, Boolean.TRUE);
        queryAls.put(DISPLAY_NAME, hasDisplayName);
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
        return getImdb().getCollection("entity");
    }

    private static User extractValidUserFromAlias(DBObject aliasObj) {
        if (aliasObj == null) {
            return null;
        }
        User user = new User();
        String id = getStringValue(aliasObj, ALIAS_ID);
        user.setIdentity(id);
        user.setUserName(id);
        user.setUri(getStringValue(aliasObj, CONTACT));
        user.setInDirectory(false);
        return user;
    }

    private static User extractValidUser(DBObject obj) {
        if (obj == null) {
            return null;
        }
        if (!Boolean.valueOf(obj.get(VALID_USER).toString())) {
            return null;
        }
        return extractUser(obj);
    }

    private static User extractUser(DBObject obj) {
        if (obj == null) {
            return null;
        }

        User user = new User();
        user.setIdentity(getStringValue(obj, IDENTITY));
        user.setUserName(getStringValue(obj, UID));
        user.setDisplayName(getStringValue(obj, DISPLAY_NAME));
        user.setUri(getStringValue(obj, CONTACT));
        user.setPasstoken(getStringValue(obj, HASHED_PASSTOKEN));
        user.setPintoken(getStringValue(obj, PINTOKEN));

        BasicDBList permissions = (BasicDBList) obj.get(PERMISSIONS);
        if (permissions != null) {
            user.setInDirectory(permissions.contains(IMDB_PERM_AA));
            user.setHasVoicemail(permissions.contains(IMDB_PERM_VOICEMAIL));
            user.setCanRecordPrompts(permissions.contains(IMDB_PERM_RECPROMPTS));
            user.setCanTuiChangePin(permissions.contains(IMDB_PERM_TUICHANGEPIN));
        }

        user.setUserBusyPrompt(Boolean.valueOf(getStringValue(obj, USERBUSYPROMPT)));
        user.setVoicemailTui(getStringValue(obj, VOICEMAILTUI));
        user.setEmailAddress(getStringValue(obj, EMAIL));
        if (obj.keySet().contains(NOTIFICATION)) {
            user.setEmailFormat(getStringValue(obj, NOTIFICATION));
        }
        user.setAttachAudioToEmail(Boolean.valueOf(getStringValue(obj, ATTACH_AUDIO)));

        user.setAltEmailAddress(getStringValue(obj, ALT_EMAIL));
        if (obj.keySet().contains(ALT_NOTIFICATION)) {
            user.setAltEmailFormat(getStringValue(obj, ALT_NOTIFICATION));
        }
        user.setAltAttachAudioToEmail(Boolean.valueOf(getStringValue(obj, ALT_ATTACH_AUDIO)));

        BasicDBList aliasesObj = (BasicDBList) obj.get(ALIASES);
        if (aliasesObj != null) {
            Vector<String> aliases = new Vector<String>();
            for (int i = 0; i < aliasesObj.size(); i++) {
                DBObject aliasObj = (DBObject) aliasesObj.get(i);
                if (aliasObj.get(RELATION).toString().equals(ALIAS)) {
                    aliases.add(aliasObj.get(ALIAS_ID).toString());
                }
            }
            user.setAliases(aliases);
        }

        if (obj.keySet().contains(SYNC)) {
            ImapInfo imapInfo = new ImapInfo();
            imapInfo.setSynchronize(Boolean.valueOf(getStringValue(obj, SYNC)));
            imapInfo.setHost(getStringValue(obj, HOST));
            imapInfo.setPort(getStringValue(obj, PORT));
            imapInfo.setUseTLS(Boolean.valueOf(getStringValue(obj, TLS)));
            imapInfo.setAccount(getStringValue(obj, ACCOUNT));
            imapInfo.setPassword(getStringValue(obj, PASSWD));
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

        // contact info related data
        user.setCellNum(getStringValue(obj, CELL_PHONE_NUMBER));
        user.setHomeNum(getStringValue(obj, HOME_PHONE_NUMBER));
        user.setConfEntryIM(getStringValue(obj, CONF_ENTRY_IM));
        user.setConfExitIM(getStringValue(obj, CONF_EXIT_IM));
        user.setVMEntryIM(getStringValue(obj, LEAVE_MESSAGE_BEGIN_IM));
        user.setVMExitIM(getStringValue(obj, LEAVE_MESSAGE_END_IM));
        user.setJid(getStringValue(obj, IM_ID));
        user.setAltJid(getStringValue(obj, ALT_IM_ID));
        user.setImPassword(getStringValue(obj, IM_PASSWORD));
        user.setOnthePhoneMessage(getStringValue(obj, IM_ON_THE_PHONE_MESSAGE));
        user.setCompanyName(getStringValue(obj, COMPANY_NAME));
        user.setJobDepartment(getStringValue(obj, JOB_DEPT));
        user.setJobTitle(getStringValue(obj, JOB_TITLE));
        user.setFaxNumber(getStringValue(obj, FAX_NUMBER));

        // office details
        user.setOfficeStreet(getStringValue(obj, OFFICE_STREET));
        user.setOfficeCity(getStringValue(obj, OFFICE_CITY));
        user.setOfficeState(getStringValue(obj, OFFICE_STATE));
        user.setOfficeZip(getStringValue(obj, OFFICE_ZIP));
        user.setOfficeCountry(getStringValue(obj, OFFICE_COUNTRY));

        // home details
        user.setHomeCity(getStringValue(obj, HOME_CITY));
        user.setHomeState(getStringValue(obj, HOME_STATE));
        user.setHomeZip(getStringValue(obj, HOME_ZIP));
        user.setHomeCountry(getStringValue(obj, HOME_COUNTRY));
        user.setHomeStreet(getStringValue(obj, HOME_STREET));

        user.setAvatar(getStringValue(obj, AVATAR));

        // active greeting related data
        if (obj.keySet().contains(ACTIVEGREETING)) {
            user.setActiveGreeting(getStringValue(obj, ACTIVEGREETING));
        }

        // personal attendant related data
        if (obj.keySet().contains(PERSONAL_ATT)) {
            BasicDBObject pao = (BasicDBObject) obj.get(PERSONAL_ATT);
            String operator = getStringValue(pao, OPERATOR);
            String language = getStringValue(pao, LANGUAGE);
            Map<String, String> menu = new HashMap<String, String>();
            StringBuilder validDigits = new StringBuilder(10);
            BasicDBList buttonsList = (BasicDBList) pao.get(BUTTONS);
            if (buttonsList != null) {
                for (int i = 0; i < buttonsList.size(); i++) {
                    DBObject button = (DBObject) buttonsList.get(i);
                    if (button != null) {
                        menu.put(getStringValue(button, DIALPAD), getStringValue(button, ITEM));
                        validDigits.append(getStringValue(button, DIALPAD));
                    }
                }
            }
            user.setPersonalAttendant(new PersonalAttendant(language, operator, menu, validDigits.toString()));
        }

        // distribution lists
        if (obj.keySet().contains(DISTRIB_LISTS)) {
            Distributions distribs = new Distributions();
            BasicDBList distribList = (BasicDBList) obj.get(DISTRIB_LISTS);
            if (distribList != null) {
                for (int i = 0; i < distribList.size(); i++) {
                    DBObject distrib = (DBObject) distribList.get(i);
                    if (distrib != null) {
                        distribs.addList(getStringValue(distrib, DIALPAD),
                                StringUtils.split(getStringValue(distrib, ITEM), " "));
                    }
                }
            }
            user.setDistributions(distribs);
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
    protected static String compress(String orig) {
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
    protected static String mapDTMF(String orig) {
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
    protected static void buildDialPatterns(User u) {
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

    public DB getImdb() {
        return m_imdb;
    }

    public void setImdb(DB imdb) {
        m_imdb = imdb;
    }
}
