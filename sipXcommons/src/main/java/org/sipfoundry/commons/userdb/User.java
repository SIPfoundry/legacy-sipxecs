/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.userdb;

import java.util.HashMap;
import java.util.Locale;
import java.util.Vector;

import org.apache.commons.codec.digest.DigestUtils;
import org.apache.commons.lang.StringUtils;

public class User {
    private String m_sysId;
    private String m_identity;
    private String m_userName;
    private String m_displayName;
    private String m_uri;
    private String m_pintoken;
    private String m_voicemailPintoken;
    private String m_passtoken;
    private boolean m_inDirectory;
    private boolean m_hasVoicemail;
    private boolean m_userBusyPrompt;
    private String m_voicemailTui;
    private boolean m_canRecordPrompts;
    private boolean m_canTuiChangePin;
    private Vector<String> m_dialPatterns;
    private Vector<String> m_aliases;
    private HashMap<String, DistributionList> m_distributionLists;
    private Locale m_locale; // The locale for the UI to present to this user
    private String m_emailAddress;
    private String m_altEmailAddress;
    private boolean m_attachAudioToEmail;
    private boolean m_altAttachAudioToEmail;
    private ImapInfo m_imapInfo;
    private String m_cellNumber;
    private String m_homeNumber;
    private boolean m_imEnabled;
    private String m_jid;
    private String m_altjid;
    private String m_confName;
    private String m_confNum;
    private String m_confPin;
    private boolean m_sendConfEntryIM;
    private boolean m_sendConfExitIM;
    private boolean m_sendVMEntryIM;
    private boolean m_sendVMExitIM;
    private boolean m_callIM;
    private boolean m_callFromAnyIM;
    private String m_onthePhoneMessage;
    private boolean m_advertiseOnCallStatus;
    private boolean m_showOnCallDetails;
    private boolean m_playDefaultVmOption;
    private String m_imDisplayName;
    private String m_activeGreeting;
    private String m_companyName;
    private String m_jobDepartment;
    private String m_jobTitle;
    private String m_faxNumber;
    private String m_officeStreet;
    private String m_officeCity;
    private String m_officeState;
    private String m_officeZip;
    private String m_officeCountry;
    private String m_homeStreet;
    private String m_homeCity;
    private String m_homeState;
    private String m_homeZip;
    private String m_homeCountry;
    private String m_avatar;
    private PersonalAttendant m_personalAttendant;
    private Distributions m_distributions;
    private String m_moh;

    public enum EmailFormats {
        FORMAT_NONE("NONE"), FORMAT_FULL("FULL"), FORMAT_MEDIUM("MEDIUM"), FORMAT_BRIEF("BRIEF"), FORMAT_IMAP("IMAP");
        private String m_id;

        EmailFormats(String id) {
            m_id = id;
        }

        public String getId() {
            return m_id;
        }

        public static EmailFormats valueOfById(String id) {
            for (EmailFormats ef : EmailFormats.values()) {
                if (ef.getId().equals(id)) {
                    return ef;
                }
            }
            throw new IllegalArgumentException("id not recognized " + id);
        }
    };

    private EmailFormats m_emailFormat = EmailFormats.FORMAT_NONE;
    private EmailFormats m_altEmailFormat = EmailFormats.FORMAT_NONE;

    public String getIdentity() {
        return m_identity;
    }

    public void setIdentity(String identity) {
        m_identity = identity;
    }

    public String getUserName() {
        return m_userName;
    }

    public void setUserName(String userName) {
        m_userName = userName;
    }

    public String getDisplayName() {
        return m_displayName;
    }

    public void setDisplayName(String displayName) {
        m_displayName = displayName;
    }

    public String getUri() {
        return m_uri;
    }

    public void setUri(String uri) {
        m_uri = uri;
    }

    public String getPintoken() {
        return m_pintoken;
    }

    public void setPintoken(String pintoken) {
        m_pintoken = pintoken;
    }

    public String getVoicemailPintoken() {
        return m_voicemailPintoken;
    }

    public void setVoicemailPintoken(String voicemailPintoken) {
        m_voicemailPintoken = voicemailPintoken;
    }

    public void setPasstoken(String passtoken) {
        m_passtoken = passtoken;
    }

    public String getPasstoken() {
        return m_passtoken;
    }

    public boolean isInDirectory() {
        return m_inDirectory;
    }

    public void setInDirectory(boolean inDirectory) {
        m_inDirectory = inDirectory;
    }

    public boolean hasVoicemail() {
        return m_hasVoicemail;
    }

    public void setHasVoicemail(boolean hasVoicmail) {
        m_hasVoicemail = hasVoicmail;
    }

    public boolean userBusyPrompt() {
        return m_userBusyPrompt;
    }

    public void setUserBusyPrompt(boolean userBusyPrompt) {
        m_userBusyPrompt = userBusyPrompt;
    }

    public void setVoicemailTui(String voicemailTui) {
        m_voicemailTui = voicemailTui;
    }

    public String getVoicemailTui() {
        return m_voicemailTui;
    }

    public boolean canRecordPrompts() {
        return m_canRecordPrompts;
    }

    public void setCanRecordPrompts(boolean canRecordPrompts) {
        m_canRecordPrompts = canRecordPrompts;
    }

    public boolean canTuiChangePin() {
        return m_canTuiChangePin;
    }

    public void setCanTuiChangePin(boolean canTuiChangePin) {
        m_canTuiChangePin = canTuiChangePin;
    }

    public Vector<String> getDialPatterns() {
        return m_dialPatterns;
    }

    public void setDialPatterns(Vector<String> dialPatterns) {
        this.m_dialPatterns = dialPatterns;
    }

    public HashMap<String, DistributionList> getDistributionLists() {
        return m_distributionLists;
    }

    public void setDistributionLists(HashMap<String, DistributionList> distributionLists) {
        this.m_distributionLists = distributionLists;
    }

    public Vector<String> getAliases() {
        return m_aliases;
    }

    public void setAliases(Vector<String> aliases) {
        this.m_aliases = aliases;
    }

    public String hashPin(String pin) {
        // pintoken is MD5 Hash of:
        // userName:pin
        String token = m_userName + ":" + pin;
        return DigestUtils.md5Hex(token);
    }

    public boolean isVoicemailPinCorrect(String pin) {
        return m_voicemailPintoken.equals(hashPin(pin));
    }

    public Locale getLocale() {
        return m_locale;
    }

    public void setLocale(Locale locale) {
        m_locale = locale;
    }

    public String getEmailAddress() {
        return m_emailAddress;
    }

    public void setEmailAddress(String emailAddress) {
        m_emailAddress = emailAddress;
    }

    public String getAltEmailAddress() {
        return m_altEmailAddress;
    }

    public void setAltEmailAddress(String emailAddress) {
        m_altEmailAddress = emailAddress;
    }

    public boolean isAttachAudioToEmail() {
        return m_attachAudioToEmail;
    }

    public void setAttachAudioToEmail(boolean attachAudioToEmail) {
        m_attachAudioToEmail = attachAudioToEmail;
    }

    public boolean isAltAttachAudioToEmail() {
        return m_altAttachAudioToEmail;
    }

    public void setAltAttachAudioToEmail(boolean altAttachAudioToEmail) {
        m_altAttachAudioToEmail = altAttachAudioToEmail;
    }

    public EmailFormats getEmailFormat() {
        return m_emailFormat;
    }

    public void setEmailFormat(EmailFormats emailFormat) {
        m_emailFormat = emailFormat;
    }

    public void setEmailFormat(String emailFormat) {
        m_emailFormat = EmailFormats.valueOfById(emailFormat);
    }

    public EmailFormats getAltEmailFormat() {
        return m_altEmailFormat;
    }

    public void setAltEmailFormat(EmailFormats emailFormat) {
        m_altEmailFormat = emailFormat;
    }

    public void setAltEmailFormat(String emailFormat) {
        m_altEmailFormat = EmailFormats.valueOfById(emailFormat);
    }

    public ImapInfo getImapInfo() {
        return m_imapInfo;
    }

    public void setImapInfo(ImapInfo imapInfo) {
        m_imapInfo = imapInfo;
    }

    public void setCellNum(String cellNum) {
        m_cellNumber = cellNum;
    }

    public String getCellNum() {
        return m_cellNumber;
    }

    public String getHomeNum() {
        return m_homeNumber;
    }

    public void setHomeNum(String homeNum) {
        m_homeNumber = homeNum;
    }

    public String getJid() {
        return m_jid;
    }

    public void setJid(String jid) {
        m_jid = jid;
    }

    public String getAltJid() {
        return m_altjid;
    }

    public void setAltJid(String jid) {
        m_altjid = jid;
    }

    public void setConfName(String confName) {
        m_confName = confName;
    }

    public String getConfName() {
        return m_confName;
    }

    public void setConfNum(String confNum) {
        m_confNum = confNum;
    }

    public String getConfNum() {
        return m_confNum;
    }

    public void setConfPin(String confPin) {
        m_confPin = confPin;
    }

    public String getConfPin() {
        return m_confPin;
    }

    public void setConfEntryIM(String value) {
        if (value != null) {
            m_sendConfEntryIM = value.equals("1") || value.equals("true");
        }
    }

    public boolean getConfEntryIM() {
        return m_sendConfEntryIM;
    }

    public void setConfExitIM(String value) {
        if (value != null) {
            m_sendConfExitIM = value.equals("1") || value.equals("true");
        }
    }

    public boolean getConfExitIM() {
        return m_sendConfExitIM;
    }

    public void setVMEntryIM(String value) {
        if (value != null) {
            m_sendVMEntryIM = value.equals("1") || value.equals("true");
        }
    }

    public boolean getVMEntryIM() {
        return m_sendVMEntryIM;
    }

    public void setVMExitIM(String value) {
        if (value != null) {
            m_sendVMExitIM = value.equals("1") || value.equals("true");
        }
    }

    public boolean getVMExitIM() {
        return m_sendVMExitIM;
    }

    public boolean isCallIM() {
        return m_callIM;
    }

    public void setCallIM(String value) {
        if (value != null) {
            m_callIM = value.equals("1") || value.equals("true");
        }
    }

    public boolean isCallFromAnyIM() {
        return m_callFromAnyIM;
    }

    public void setCallFromAnyIM(String value) {
        if (value != null) {
            m_callFromAnyIM = value.equals("1") || value.equals("true");
        }
    }

    public String getOnthePhoneMessage() {
        return m_onthePhoneMessage;
    }

    public void setOnthePhoneMessage(String onthePhoneMessage) {
        m_onthePhoneMessage = onthePhoneMessage;
    }

    public boolean isAdvertiseOnCallStatus() {
        return m_advertiseOnCallStatus;
    }

    public void setAdvertiseOnCallStatus(boolean advertiseOnCallStatus) {
        m_advertiseOnCallStatus = advertiseOnCallStatus;
    }

    public boolean isShowOnCallDetails() {
        return m_showOnCallDetails;
    }

    public void setShowOnCallDetails(boolean showOnCallDetails) {
        m_showOnCallDetails = showOnCallDetails;
    }

    public boolean isImEnabled() {
        return m_imEnabled;
    }

    public void setImEnabled(boolean enabled) {
        m_imEnabled = enabled;
    }

    public String getImDisplayName() {
        return m_imDisplayName;
    }

    public void setImDisplayName(String imDisplayName) {
        m_imDisplayName = imDisplayName;
    }

    public void setActiveGreeting(String greeting) {
        m_activeGreeting = greeting;
    }

    public String getActiveGreeting() {
        return m_activeGreeting;
    }

    public void setPersonalAttendant(PersonalAttendant pa) {
        m_personalAttendant = pa;
    }

    public PersonalAttendant getPersonalAttendant() {
        if (m_personalAttendant == null) {
            m_personalAttendant = new PersonalAttendant(null, null, new HashMap<String, String>(), StringUtils.EMPTY);
        }
        return m_personalAttendant;
    }

    public void setDistributions(Distributions distribs) {
        m_distributions = distribs;
    }

    public Distributions getDistributions() {
        if (m_distributions == null) {
            m_distributions = new Distributions();
        }
        return m_distributions;
    }

    public void setCompanyName(String company) {
        m_companyName = company;
    }

    public String getCompanyName() {
        return m_companyName;
    }

    public String getJobDepartment() {
        return m_jobDepartment;
    }

    public void setJobDepartment(String jobDepartment) {
        m_jobDepartment = jobDepartment;
    }

    public String getJobTitle() {
        return m_jobTitle;
    }

    public void setJobTitle(String jobTitle) {
        m_jobTitle = jobTitle;
    }
    public String getFaxNumber() {
        return m_faxNumber;
    }

    public void setFaxNumber(String faxNumber) {
        m_faxNumber = faxNumber;
    }

    public String getOfficeStreet() {
        return m_officeStreet;
    }

    public void setOfficeStreet(String officeStreet) {
        m_officeStreet = officeStreet;
    }

    public String getOfficeCity() {
        return m_officeCity;
    }

    public void setOfficeCity(String officeCity) {
        m_officeCity = officeCity;
    }

    public String getOfficeState() {
        return m_officeState;
    }

    public void setOfficeState(String officeState) {
        m_officeState = officeState;
    }

    public String getOfficeZip() {
        return m_officeZip;
    }

    public void setOfficeZip(String officeZip) {
        m_officeZip = officeZip;
    }

    public String getOfficeCountry() {
        return m_officeCountry;
    }

    public void setOfficeCountry(String officeCountry) {
        m_officeCountry = officeCountry;
    }

    public String getHomeStreet() {
        return m_homeStreet;
    }

    public void setHomeStreet(String homeStreet) {
        m_homeStreet = homeStreet;
    }

    public String getHomeCity() {
        return m_homeCity;
    }

    public void setHomeCity(String homeCity) {
        m_homeCity = homeCity;
    }

    public String getHomeState() {
        return m_homeState;
    }

    public void setHomeState(String homeState) {
        m_homeState = homeState;
    }

    public String getHomeZip() {
        return m_homeZip;
    }

    public void setHomeZip(String homeZip) {
        m_homeZip = homeZip;
    }

    public String getHomeCountry() {
        return m_homeCountry;
    }

    public void setHomeCountry(String homeCountry) {
        m_homeCountry = homeCountry;
    }

    public String getAvatar() {
        return m_avatar;
    }

    public void setAvatar(String avatar) {
        m_avatar = avatar;
    }

    public String getSysId() {
        return m_sysId;
    }

    public void setSysId(String id) {
        m_sysId = id;
    }

    public boolean shouldPlayDefaultVmOption() {
        return m_playDefaultVmOption;
    }

    public void setPlayDefaultVmOption(boolean playDefaultOption) {
        m_playDefaultVmOption = playDefaultOption;
    }

    public String getMoh() {
        return m_moh;
    }

    public void setMoh(String moh) {
        m_moh = moh;
    }

    public String getFirstName() {
        if (m_displayName != null) {
            String[] nameTokens = StringUtils.split(m_displayName, " ");
            if (nameTokens != null && nameTokens.length > 0) {
                return StringUtils.split(m_displayName, " ")[0];
            }
        }
        return StringUtils.EMPTY;
    }

    public String getLastName() {
        if (m_displayName != null) {
            String[] nameTokens = StringUtils.split(m_displayName, " ");
            if (nameTokens != null && nameTokens.length > 1) {
                return StringUtils.split(m_displayName, " ")[1];
            }
        }
        return StringUtils.EMPTY;
    }
}
