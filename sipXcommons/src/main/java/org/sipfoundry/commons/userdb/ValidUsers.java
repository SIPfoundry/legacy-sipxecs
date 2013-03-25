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

import static org.sipfoundry.commons.mongo.MongoConstants.*;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
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
import com.mongodb.WriteResult;
import com.mongodb.util.JSON;

/**
 * Holds the valid user data needed for the AutoAttendant, parsing from mongo db imdb
 *
 */
public class ValidUsers {
    public static String IM_USERNAME_FILTER = "Username";
    public static String IM_NAME_FILTER = "Name";
    public static String IM_EMAIL_FILTER = "Email";
    // Mapping of letters to DTMF numbers.
    // Position of letter in letters maps to corresponding position in numbers
    private static String LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    private static String NUMBERS = "22233344455566677778889999";
    private static final String IMDB_PERM_AA = "AutoAttendant";
    private static final String IMDB_PERM_VOICEMAIL = "Voicemail";
    private static final String IMDB_PERM_RECPROMPTS = "RecordSystemPrompts";
    private static final String IMDB_PERM_TUICHANGEPIN = "tui-change-pin";
    private static final String ENTITY_NAME_USER = "user";
    private static final String ENTITY_NAME_GROUP = "group";
    private static final String ENTITY_NAME_IMBOTSETTINGS = "imbotsettings";

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
        BasicDBObject query = new BasicDBObject(ENTITY_NAME, ENTITY_NAME_USER);
        DBCursor cursor = getEntityCollection().find(query);
        return cursor;
    }

    public DBCursor getUsersWithSpeedDial() {
        DBCursor cursor = getEntityCollection().find(
                QueryBuilder.start(ENTITY_NAME).is(ENTITY_NAME_USER).
                or(
                        QueryBuilder.start(SPEEDDIAL).exists(true).get(),
                        new BasicDBObject(IM_ENABLED, true)
                  ).get());
        return cursor;
    }

    /**
     * Use this method if you need to remove a field from users completely.
     * This may be achieved by regenerating the entire collection, or a DataSet, but this would be much faster.
     * @param field
     */
    public void removeFieldFromUsers(String field) {
        getEntityCollection().update( new BasicDBObject()
        , new BasicDBObject( "$unset" , new BasicDBObject( SPEEDDIAL , 1 ) )
        , false , true ); 
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
        return getUserByJidObject(jid);
    }

    //We might rarely need to search through alternate id. "or" query proved to be pretty heavy
    //especially on systems with many users, so we might want to limit those if possible
    private User getUserByJidObject(Object jid) {
        BasicDBObject jidQuery = new BasicDBObject();
        jidQuery.put(IM_ID, jid);
        DBObject jidResult = getEntityCollection().findOne(jidQuery);
        User user = extractValidUser(jidResult);
        if (user == null) {
            BasicDBObject altJidQuery = new BasicDBObject();
            altJidQuery.put(ALT_IM_ID, jid);
            jidResult = getEntityCollection().findOne(altJidQuery);
            user = extractValidUser(jidResult);
        }
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
        DBObject query = QueryBuilder.start(ENTITY_NAME).is(ENTITY_NAME_USER).and(MongoConstants.TIMESTAMP).greaterThan(ms).get();
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

    public long getImUsersCount() {
        return getEntityCollection().count(QueryBuilder.start(IM_ENABLED).is(Boolean.TRUE).get());
    }

    public List<User> getImUsersByFilter(Set<String> fields, String query, int startIndex, int numResults) {
        QueryBuilder mongoQuery = QueryBuilder.start();
        if (fields.contains(IM_USERNAME_FILTER)) {
            BasicDBObject q = new BasicDBObject();
            q.put(IM_ID, query);
            BasicDBObject altQ = new BasicDBObject();
            altQ.put(ALT_IM_ID, query);
            mongoQuery.or(q, altQ);
        }
        if (fields.contains(IM_NAME_FILTER)) {
            BasicDBObject q = new BasicDBObject();
            q.put(IM_DISPLAY_NAME, query);
            mongoQuery.or(q);
        }
        if (fields.contains(IM_EMAIL_FILTER)) {
            BasicDBObject q = new BasicDBObject();
            q.put(EMAIL, query);
            mongoQuery.or(q);
        }
        DBCursor cursor = getEntityCollection().find(mongoQuery.get()).skip(startIndex).limit(numResults);
        List<User> users = new ArrayList<User>();
        Iterator<DBObject> objects = cursor.iterator();
        while (objects.hasNext()) {
            DBObject user = objects.next();
            User imUser = extractUser(user);
            if (imUser.isImEnabled()) {
                users.add(imUser);
            }
        }
        return users;
    }

    public Collection<String> getImUsernames(int startIndex, int numResults) {
        BasicDBObject query = new BasicDBObject();
        query.put(IM_ENABLED, true);
        DBObject dbObject = (DBObject) JSON.parse("{'" + IM_ID + "':1}");
        List<String> userNames = new ArrayList<String>();
        DBCursor cursor = getEntityCollection().find(query, dbObject).skip(startIndex).limit(numResults);
        Iterator<DBObject> objects = cursor.iterator();
        while (objects.hasNext()) {
            DBObject user = objects.next();
            String imUsername = getStringValue(user, IM_ID);
            if (StringUtils.isNotBlank(imUsername)) {
                userNames.add(imUsername);
            }
        }
        return userNames;
    }

    public UserGroup getImGroup(String name) {
        DBObject queryGroup = QueryBuilder.start(IM_GROUP).is("1").and(UID).is(name).get();
        DBObject groupResult = getEntityCollection().findOne(queryGroup);
        if (groupResult != null) {
            return convertUserGroup(groupResult);
        }
        return null;
    }

    public Collection<UserGroup> getImGroups() {
        Collection<UserGroup> groups = new ArrayList<UserGroup> ();
        DBObject queryGroup = QueryBuilder.start(IM_GROUP).is("1").get();
        DBCursor cursor = getEntityCollection().find(queryGroup);
        DBObject groupResult = null;
        while (cursor.hasNext()) {
            groupResult = cursor.next();
            if (groupResult != null) {
                groups.add(convertUserGroup(groupResult));
            }
        }
        return groups;
    }

    private UserGroup convertUserGroup(DBObject groupResult) {
        UserGroup group = new UserGroup();
        group.setGroupName(getStringValue(groupResult, UID));
        group.setDescription(getStringValue(groupResult, DESCR));
        group.setSysId(getStringValue(groupResult, ID));
        String imBotEnabled = getStringValue(groupResult, MY_BUDDY_GROUP);
        if (StringUtils.equals(imBotEnabled, "1")) {
            group.setImBotEnabled(true);
        }
        return group;
    }

    public String getImBotName() {
        User imbotUser = getImbotUser();
        if (imbotUser != null) {
            return imbotUser.getUserName();
        }
        return null;
    }

    public User getImbotUser() {
        DBObject queryImbot = QueryBuilder.start(ENTITY_NAME).is(ENTITY_NAME_IMBOTSETTINGS).and(IM_ENABLED).is(true).get();
        DBObject imbotResult = getEntityCollection().findOne(queryImbot);
        if (imbotResult != null) {
            User imbotUser = new User();
            imbotUser.setUserName(getStringValue(imbotResult, IM_ID));
            imbotUser.setPintoken(getStringValue(imbotResult, PINTOKEN));
            return imbotUser;
        }

        return null;
    }

    public long getImGroupCount() {
        return getEntityCollection().count(QueryBuilder.start(ENTITY_NAME).is(ENTITY_NAME_GROUP).and(IM_GROUP).is("1").get());
    }

    public Collection<String> getImGroupNames(int startIndex, int numResults) {
        BasicDBObject query = new BasicDBObject();
        query.put(IM_GROUP, "1");
        DBObject dbObject = (DBObject) JSON.parse("{'" + UID + "':1}");
        List<String> groupNames = new ArrayList<String>();
        DBCursor cursor = getEntityCollection().find(query, dbObject).skip(startIndex).limit(numResults);
        Iterator<DBObject> objects = cursor.iterator();
        while (objects.hasNext()) {
            DBObject group = objects.next();
            String groupName = getStringValue(group, UID);
            if (StringUtils.isNotBlank(groupName)) {
                groupNames.add(groupName);
            }
        }
        return groupNames;
    }

    public List<String> getImGroupNameByQuery(String query, int startIndex, int numResults) {
        Pattern insensitiveQuery = Pattern.compile(query, Pattern.CASE_INSENSITIVE);
        QueryBuilder mongoQuery = QueryBuilder.start(ENTITY_NAME).is(ENTITY_NAME_GROUP).and(UID).is(insensitiveQuery);
        DBCursor cursor = getEntityCollection().find(mongoQuery.get()).skip(startIndex).limit(numResults);
        List<String> groups = new ArrayList<String>();
        Iterator<DBObject> objects = cursor.iterator();
        while (objects.hasNext()) {
            DBObject group = objects.next();
            groups.add(getStringValue(group, UID));
        }
        return groups;
    }

    public List<String> getImGroupnamesForUser(String jid) {
        BasicDBObject query = new BasicDBObject();
        query.put(IM_ENABLED, true);
        query.put(IM_ID, jid);
        List<String> names = new ArrayList<String>();
        DBObject user = getEntityCollection().findOne(query);
        if (user != null) {
            BasicDBList groupList = (BasicDBList) user.get(GROUPS);
            for (int i = 0; i < groupList.size(); i++) {
                names.add((String) groupList.get(i));
            }
        }
        return names;
    }

    public List<String> getImUsernamesInGroup(String groupName) {
        BasicDBObject query = new BasicDBObject();
        query.put(IM_ENABLED, true);
        BasicDBList groupList = new BasicDBList();
        groupList.add(groupName);
        query.put(GROUPS, new BasicDBObject("$in", groupList));
        DBObject dbObject = (DBObject) JSON.parse("{'" + IM_ID + "':1}");
        List<String> userNames = new ArrayList<String>();
        DBCursor cursor = getEntityCollection().find(query, dbObject);
        Iterator<DBObject> objects = cursor.iterator();
        while (objects.hasNext()) {
            DBObject user = objects.next();
            String imUsername = getStringValue(user, IM_ID);
            if (StringUtils.isNotBlank(imUsername)) {
                userNames.add(imUsername);
            }
        }
        return userNames;
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
        user.setSysId(getStringValue(obj, ID));
        user.setIdentity(getStringValue(obj, IDENTITY));
        user.setUserName(getStringValue(obj, UID));
        user.setDisplayName(getStringValue(obj, DISPLAY_NAME));
        user.setUri(getStringValue(obj, CONTACT));
        user.setPasstoken(getStringValue(obj, HASHED_PASSTOKEN));
        user.setPintoken(getStringValue(obj, PINTOKEN));
        user.setVoicemailPintoken(getStringValue(obj, VOICEMAIL_PINTOKEN));

        BasicDBList permissions = (BasicDBList) obj.get(PERMISSIONS);
        if (permissions != null) {
            user.setInDirectory(permissions.contains(IMDB_PERM_AA));
            user.setHasVoicemail(permissions.contains(IMDB_PERM_VOICEMAIL));
            user.setCanRecordPrompts(permissions.contains(IMDB_PERM_RECPROMPTS));
            user.setCanTuiChangePin(permissions.contains(IMDB_PERM_TUICHANGEPIN));
        }

        user.setUserBusyPrompt(Boolean.valueOf(getStringValue(obj, USERBUSYPROMPT)));
        user.setMoh(getStringValue(obj, MOH));
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
        user.setCallIM(getStringValue(obj, CALL_IM));
        user.setCallFromAnyIM(getStringValue(obj, CALL_FROM_ANY_IM));
        user.setImEnabled(Boolean.valueOf(getStringValue(obj, IM_ENABLED)));
        user.setJid(getStringValue(obj, IM_ID));
        user.setAltJid(getStringValue(obj, ALT_IM_ID));
        user.setImDisplayName(getStringValue(obj, IM_DISPLAY_NAME));
        user.setOnthePhoneMessage(getStringValue(obj, IM_ON_THE_PHONE_MESSAGE));
        user.setAdvertiseOnCallStatus(Boolean.valueOf(getStringValue(obj, IM_ADVERTISE_ON_CALL_STATUS)));
        user.setShowOnCallDetails(Boolean.valueOf(getStringValue(obj, IM_SHOW_ON_CALL_DETAILS)));
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

        user.setPlayDefaultVmOption(Boolean.valueOf(getStringValue(obj, PLAY_DEFAULT_VM)));

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
        normal = normal.replaceAll("[\u00C0\u00C1\u00C2\u00C3\u00C4]", "A");
        normal = normal.replaceAll("[\u00C8\u00C9\u00CA\u00CB]", "E");
        normal = normal.replaceAll("[\u00CC\u00CD\u00CE\u00CF]", "I");
        normal = normal.replaceAll("[\u00D2\u00D3\u00D4\u00D5\u00D6]", "O");
        normal = normal.replaceAll("[\u00D9\u00DA\u00DB\u00DC]", "U");
        normal = normal.replaceAll("\u00C7", "C");
        normal = normal.replaceAll("\u00D1", "N");

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
