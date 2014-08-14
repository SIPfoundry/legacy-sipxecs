/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 */
package org.sipfoundry.openfire.provider;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.StringReader;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathFactory;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringEscapeUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;
import org.jivesoftware.openfire.provider.VCardProvider;
import org.jivesoftware.util.AlreadyExistsException;
import org.jivesoftware.util.NotFoundException;
import org.sipfoundry.commons.mongo.MongoFactory;
import org.sipfoundry.commons.userdb.profile.Address;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.commons.userdb.profile.UserProfileService;
import org.sipfoundry.commons.userdb.profile.UserProfileServiceImpl;
import org.sipfoundry.openfire.vcard.ContactInfoHandlerImpl;
import org.sipfoundry.openfire.vcard.synchserver.VCardRpcServer;
import org.springframework.data.mongodb.core.MongoTemplate;

import com.mongodb.Mongo;

public class MongoVCardProvider implements VCardProvider {
    private static Logger logger = Logger.getLogger(MongoVCardProvider.class);
    private UserProfileService m_userProfileService;
    private final XPath m_xpath = XPathFactory.newInstance().newXPath();

    public MongoVCardProvider() throws Exception {
        initUserProfileService();
        startVCardSynchServer();
    }

    private void initUserProfileService() throws Exception {
        Mongo mongo = MongoFactory.fromConnectionFile();
        MongoTemplate profilesDb = new MongoTemplate(mongo, "profiles");
        m_userProfileService = new UserProfileServiceImpl();
        ((UserProfileServiceImpl) m_userProfileService).setProfilesDb(profilesDb);
    }

    private void startVCardSynchServer() {
        logger.info(this.getClass().getName() + " starting XML RPC server ...");
        try {
            VCardRpcServer vcardRpcServer = new VCardRpcServer(ContactInfoHandlerImpl.class);
            vcardRpcServer.start();
            logger.info(this.getClass().getName() + " initialized");
        } catch (Exception ex) {
            logger.error(ex);
        }
    }

    @Override
    public Element createVCard(String username, Element xcard) throws AlreadyExistsException {
        // no need to create, users are created only via sipxconfig
        return xcard;
    }

    @Override
    public void deleteVCard(String userName) {
    }

    @Override
    public boolean isReadOnly() {
        return false;
    }

    @Override
    public Element loadVCard(String userName) {
        UserProfile profile = m_userProfileService.getUserProfileByImId(userName);
        if (profile == null) {
            logger.debug(String.format("Im ID %s not known by sipxecs, returning empty VCard", userName));
            return createEmptyVCard();
        }
        return convertToVCard(profile);
    }

    private static Element createEmptyVCard() {
        StringBuilder xbuilder = new StringBuilder("<vCard xmlns='vcard-temp'>");
        xbuilder.append("</vCard>");
        return convertToElement(xbuilder.toString());
    }

    @Override
    public Element updateVCard(String userName, Element xcard) throws NotFoundException {
        UserProfile userProfile = m_userProfileService.getUserProfileByImId(userName);
        if (userProfile == null) {
            logger.warn(String.format("Im ID %s not known by sipxecs", userName));
            return xcard;
        }
        try {
            logger.debug("Updating user profile for " + userName);
            updateUserProfile(userProfile, xcard);
        } catch (Exception ex) {
            logger.error(String.format("failed to update avatar %s", ex.getMessage()));
        }
        return xcard;
    }

    private static Element convertToElement(String vcard) {
        try {
            SAXReader sreader = new SAXReader();
            Document vcardDoc = sreader.read(new StringReader(vcard));
            return vcardDoc.getRootElement();
        } catch (Exception ex) {
            logger.error(ex);
        }
        return null;
    }

    private Element convertToVCard(UserProfile profile) {
        logger.debug("convert vcard to element");
        StringBuilder xbuilder = new StringBuilder("<vCard xmlns='vcard-temp'>");
        xbuilder.append("<FN>");
        xbuilder.append(defaultValue(profile.getImDisplayName()));
        xbuilder.append("</FN>");
        xbuilder.append("<N>");
        xbuilder.append("<FAMILY>");
        xbuilder.append(defaultValue(profile.getLastName()));
        xbuilder.append("</FAMILY>");
        xbuilder.append("<GIVEN>");
        xbuilder.append(defaultValue(profile.getFirstName()));
        xbuilder.append("</GIVEN>");
        xbuilder.append("<MIDDLE/>");
        xbuilder.append("</N>");

        xbuilder.append("<NICKNAME>");
        xbuilder.append("");
        xbuilder.append("</NICKNAME>");

        xbuilder.append("<URL>");
        xbuilder.append("");
        xbuilder.append("</URL>");

        xbuilder.append("<BDAY>");
        xbuilder.append("");
        xbuilder.append("</BDAY>");

        xbuilder.append("<ORG>");
        xbuilder.append("<ORGNAME>");
        xbuilder.append(defaultValue(profile.getCompanyName()));
        xbuilder.append("</ORGNAME>");
        xbuilder.append("<ORGUNIT>");
        xbuilder.append(defaultValue(profile.getJobDept()));
        xbuilder.append("</ORGUNIT>");
        xbuilder.append("</ORG>");

        xbuilder.append("<TITLE>");
        xbuilder.append(defaultValue(profile.getJobTitle()));
        xbuilder.append("</TITLE>");

        xbuilder.append("<ROLE/>");

        xbuilder.append("<TEL>");
        xbuilder.append("<WORK/>");
        xbuilder.append("<FAX/>");
        xbuilder.append("<NUMBER>");
        xbuilder.append(defaultValue(profile.getFaxNumber()));
        xbuilder.append("</NUMBER>");
        xbuilder.append("</TEL>");

        xbuilder.append("<TEL>");
        xbuilder.append("<WORK/>");
        xbuilder.append("<MSG/>");
        xbuilder.append("<NUMBER/>");
        xbuilder.append("</TEL>");

        xbuilder.append("<ADR>");
        xbuilder.append("<WORK/>");
        xbuilder.append("<EXTADD/>");
        xbuilder.append("<STREET>");
        xbuilder.append(defaultValue(profile.getOfficeAddress().getStreet()));
        xbuilder.append("</STREET>");
        xbuilder.append("<LOCALITY>");
        xbuilder.append(defaultValue(profile.getOfficeAddress().getCity()));
        xbuilder.append("</LOCALITY>");
        xbuilder.append("<REGION>");
        xbuilder.append(defaultValue(profile.getOfficeAddress().getState()));
        xbuilder.append("</REGION>");
        xbuilder.append("<PCODE>");
        xbuilder.append(defaultValue(profile.getOfficeAddress().getZip()));
        xbuilder.append("</PCODE>");
        xbuilder.append("<CTRY>");
        xbuilder.append(defaultValue(profile.getOfficeAddress().getCountry()));
        xbuilder.append("</CTRY>");
        xbuilder.append("</ADR>");

        xbuilder.append("<TEL>");
        xbuilder.append("<HOME/>");
        xbuilder.append("<VOICE/>");
        xbuilder.append("<NUMBER>");
        xbuilder.append(defaultValue(profile.getHomePhoneNumber()));
        xbuilder.append("</NUMBER>");
        xbuilder.append("</TEL>");

        xbuilder.append("<TEL>");
        xbuilder.append("<HOME/>");
        xbuilder.append("<FAX/>");
        xbuilder.append("<NUMBER/>");
        xbuilder.append("</TEL>");

        xbuilder.append("<TEL>");
        xbuilder.append("<HOME/>");
        xbuilder.append("<CELL/>");
        xbuilder.append("<NUMBER>");
        xbuilder.append(defaultValue(profile.getCellPhoneNumber()));
        xbuilder.append("</NUMBER>");
        xbuilder.append("</TEL>");

        xbuilder.append("<TEL>");
        xbuilder.append("<HOME/>");
        xbuilder.append("<MSG/>");
        xbuilder.append("<NUMBER/>");
        xbuilder.append("</TEL>");

        xbuilder.append("<ADR>");
        xbuilder.append("<HOME/>");
        xbuilder.append("<EXTADD/>");
        xbuilder.append("<STREET>");
        xbuilder.append(defaultValue(profile.getHomeAddress().getStreet()));
        xbuilder.append("</STREET>");
        xbuilder.append("<LOCALITY>");
        xbuilder.append(defaultValue(profile.getHomeAddress().getCity()));
        xbuilder.append("</LOCALITY>");
        xbuilder.append("<REGION>");
        xbuilder.append(defaultValue(profile.getHomeAddress().getState()));
        xbuilder.append("</REGION>");
        xbuilder.append("<PCODE>");
        xbuilder.append(defaultValue(profile.getHomeAddress().getZip()));
        xbuilder.append("</PCODE>");
        xbuilder.append("<CTRY>");
        xbuilder.append(defaultValue(profile.getHomeAddress().getCountry()));
        xbuilder.append("</CTRY>");
        xbuilder.append("</ADR>");

        xbuilder.append("<EMAIL>");
        xbuilder.append("<INTERNET/>");
        xbuilder.append("<PREF/>");
        xbuilder.append("<USERID>");
        xbuilder.append(defaultValue(profile.getEmailAddress()));
        xbuilder.append("</USERID>");
        xbuilder.append("</EMAIL>");

        xbuilder.append("<JABBERID>");
        xbuilder.append(defaultValue(profile.getImId()));
        xbuilder.append("</JABBERID>");

        xbuilder.append("<X-INTERN>");
        xbuilder.append(defaultValue(profile.getUserName()));
        xbuilder.append("</X-INTERN>");

        xbuilder.append("<X-DID>");
        xbuilder.append(defaultValue(profile.getDidNumber()));
        xbuilder.append("</X-DID>");

        xbuilder.append("<X-ALT-EMAIL>");
        xbuilder.append(defaultValue(profile.getAlternateEmailAddress()));
        xbuilder.append("</X-ALT-EMAIL>");

        xbuilder.append("<X-ALT-JABBERID>");
        xbuilder.append(defaultValue(profile.getAlternateImId()));
        xbuilder.append("</X-ALT-JABBERID>");

        xbuilder.append("<X-MANAGER>");
        xbuilder.append(defaultValue(profile.getManager()));
        xbuilder.append("</X-MANAGER>");

        xbuilder.append("<X-SALUTATION>");
        xbuilder.append(defaultValue(profile.getSalutation()));
        xbuilder.append("</X-SALUTATION>");

        xbuilder.append("<X-ASSISTANT>");
        xbuilder.append(defaultValue(profile.getAssistantName()));
        xbuilder.append("</X-ASSISTANT>");

        xbuilder.append("<X-ASSISTANT-PHONE>");
        xbuilder.append(defaultValue(profile.getAssistantPhoneNumber()));
        xbuilder.append("</X-ASSISTANT-PHONE>");

        xbuilder.append("<X-LOCATION>");
        xbuilder.append(defaultValue(profile.getLocation()));
        xbuilder.append("</X-LOCATION>");

        xbuilder.append("<X-TWITTER>");
        xbuilder.append(defaultValue(profile.getTwiterName()));
        xbuilder.append("</X-TWITTER>");
        xbuilder.append("<X-FACEBOOK>");
        xbuilder.append(defaultValue(profile.getFacebookName()));
        xbuilder.append("</X-FACEBOOK>");
        xbuilder.append("<X-LINKEDIN>");
        xbuilder.append(defaultValue(profile.getLinkedinName()));
        xbuilder.append("</X-LINKEDIN>");
        xbuilder.append("<X-XING>");
        xbuilder.append(defaultValue(profile.getXingName()));
        xbuilder.append("</X-XING>");
        xbuilder.append("<DESC/>");

        String encodedStr = getEncodedAvatar(defaultValue(profile.getUserName()));
        if (encodedStr != null) {
            xbuilder.append("<PHOTO>");
            xbuilder.append("<TYPE>image/png</TYPE>");
            xbuilder.append("<BINVAL>");
            xbuilder.append(encodedStr);
            xbuilder.append("</BINVAL>");
            xbuilder.append("</PHOTO>");
        }

        xbuilder.append("</vCard>");
        return convertToElement(xbuilder.toString());
    }

    private String getEncodedAvatar(String userName) {
        byte[] avatarContent = getAvatarAsByteArray(userName);
        return avatarContent != null ? new String(new Base64().encode(avatarContent)) : null;
    }

    private static String defaultValue(String value) {
        return StringUtils.defaultString(StringEscapeUtils.escapeXml(value), StringUtils.EMPTY);
    }

    @SuppressWarnings("resource")
    public byte[] getAvatarAsByteArray(String userName) {
        InputStream is = null;
        try {
            is = m_userProfileService.getAvatar(userName);
            return is != null ? IOUtils.toByteArray(is) : null;
        } catch (IOException ex) {
            return null;
        } finally {
            IOUtils.closeQuietly(is);
        }
    }

    private void updateUserProfile(UserProfile profile, Element xcard) throws Exception {
        DocumentBuilderFactory domFactory = DocumentBuilderFactory.newInstance();
        DocumentBuilder builder = domFactory.newDocumentBuilder();
        org.w3c.dom.Document vcardDoc = builder.parse(IOUtils.toInputStream(xcard.asXML()));
        profile.setLastName(extractField("//vCard/N/FAMILY/text()", vcardDoc, profile.getLastName()));
        profile.setFirstName(extractField("//vCard/N/GIVEN/text()", vcardDoc, profile.getFirstName()));
        profile.setImDisplayName(extractField("//vCard/FN/text()", vcardDoc, profile.getImDisplayName()));
        profile.setCompanyName(extractField("//vCard/ORG/ORGNAME/text()", vcardDoc, profile.getCompanyName()));
        profile.setJobDept(extractField("//vCard/ORG/ORGUNIT/text()", vcardDoc, profile.getJobDept()));
        profile.setJobTitle(extractField("//vCard/TITLE/text()", vcardDoc, profile.getJobTitle()));
        profile.setFaxNumber(extractField("//vCard/TEL[WORK][FAX]/NUMBER/text()", vcardDoc, profile.getFaxNumber()));
        profile.setHomePhoneNumber(extractField("//vCard/TEL[HOME][VOICE]/NUMBER/text()", vcardDoc,
                profile.getHomePhoneNumber()));
        profile.setCellPhoneNumber(extractField("//vCard/TEL[HOME][CELL]/NUMBER/text()", vcardDoc,
                profile.getCellPhoneNumber()));

        Address officeAddress = profile.getOfficeAddress();
        officeAddress
                .setStreet(extractField("//vCard/ADR[WORK]/STREET/text()", vcardDoc, officeAddress.getStreet()));
        officeAddress.setCity(extractField("//vCard/ADR[WORK]/LOCALITY/text()", vcardDoc, officeAddress.getCity()));
        officeAddress.setState(extractField("//vCard/ADR[WORK]/REGION/text()", vcardDoc, officeAddress.getState()));
        officeAddress.setZip(extractField("//vCard/ADR[WORK]/PCODE/text()", vcardDoc, officeAddress.getZip()));
        officeAddress
                .setCountry(extractField("//vCard/ADR[WORK]/CTRY/text()", vcardDoc, officeAddress.getCountry()));
        profile.setOfficeAddress(officeAddress);

        Address homeAddress = profile.getHomeAddress();
        homeAddress.setStreet(extractField("//vCard/ADR[HOME]/STREET/text()", vcardDoc, homeAddress.getStreet()));
        homeAddress.setCity(extractField("//vCard/ADR[HOME]/LOCALITY/text()", vcardDoc, homeAddress.getCity()));
        homeAddress.setState(extractField("//vCard/ADR[HOME]/REGION/text()", vcardDoc, homeAddress.getState()));
        homeAddress.setZip(extractField("//vCard/ADR[HOME]/PCODE/text()", vcardDoc, homeAddress.getZip()));
        homeAddress.setCountry(extractField("//vCard/ADR[HOME]/CTRY/text()", vcardDoc, homeAddress.getCountry()));
        profile.setHomeAddress(homeAddress);

        profile.setEmailAddress(extractField("//vCard/EMAIL/USERID/text()", vcardDoc, profile.getEmailAddress()));
        profile.setSalutation(extractField("//vCard/X-SALUTATION/text()", vcardDoc, profile.getSalutation()));
        profile.setDidNumber(extractField("//vCard/X-DID/text()", vcardDoc, profile.getDidNumber()));
        profile.setAlternateEmailAddress(extractField("//vCard/X-ALT-EMAIL/text()", vcardDoc,
                profile.getAlternateEmailAddress()));
        profile.setAlternateImId(extractField("//vCard/X-ALT-JABBERID/text()", vcardDoc, profile.getAlternateImId()));
        profile.setAssistantName(extractField("//vCard/X-ASSISTANT/text()", vcardDoc, profile.getAssistantName()));
        profile.setAssistantPhoneNumber(extractField("//vCard/X-ASSISTANT-PHONE/text()", vcardDoc,
                profile.getAssistantPhoneNumber()));
        profile.setAssistantPhoneNumber(extractField("//vCard/X-ASSISTANT-PHONE/text()", vcardDoc,
                profile.getAssistantPhoneNumber()));
        profile.setLocation(extractField("//vCard/X-LOCATION/text()", vcardDoc, profile.getLocation()));
        profile.setTwiterName(extractField("//vCard/X-TWITTER/text()", vcardDoc, profile.getTwiterName()));
        profile.setLinkedinName(extractField("//vCard/X-LINKEDIN/text()", vcardDoc, profile.getLinkedinName()));
        profile.setFacebookName(extractField("//vCard/X-FACEBOOK/text()", vcardDoc, profile.getFacebookName()));
        profile.setXingName(extractField("//vCard/X-XING/text()", vcardDoc, profile.getXingName()));

        m_userProfileService.saveUserProfile(profile);
        updateAvatar(profile.getUserName(), vcardDoc);
    }

    private void updateAvatar(String username, org.w3c.dom.Document xcard) {
        try {
            String encodedAvatar = m_xpath.evaluate("//vCard/PHOTO/BINVAL/text()", xcard);
            if (encodedAvatar == null) {
                return;
            }
            byte[] avtContent = new Base64().decode(encodedAvatar.getBytes());
            logger.debug("Saving avatar for: " + username);
            m_userProfileService.saveAvatar(username, new ByteArrayInputStream(avtContent), true);
        } catch (Exception ex) {
            logger.error("Cannot update avatar ", ex);
        }
    }

    private String extractField(String expression, org.w3c.dom.Document xcard, String defaultValue) {
        try {
            String value = m_xpath.evaluate(expression, xcard);

            if (StringUtils.isNotBlank(value)) {
                return value;
            }

            return defaultValue;

        } catch (Exception ex) {
            logger.error(String.format("cannot extract field from vcard using %s", expression));
            return defaultValue;
        }
    }
}
