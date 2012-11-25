/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import static org.apache.commons.lang.StringUtils.isBlank;
import static org.apache.commons.lang.StringUtils.isNotBlank;
import static org.apache.commons.lang.StringUtils.join;
import static org.sipfoundry.commons.security.Util.isHashed;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.commons.security.Md5Encoder;
import org.sipfoundry.sipxconfig.bulk.RowInserter;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserValidationUtils;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.GroupAutoAssign;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.springframework.beans.factory.annotation.Required;

public class CsvRowInserter extends RowInserter<String[]> {

    private ConferenceBridgeContext m_conferenceBridgeContext;

    private CoreContext m_coreContext;

    private ForwardingContext m_forwardingContext;

    private PhoneContext m_phoneContext;

    private SettingDao m_settingDao;

    private ModelSource<PhoneModel> m_modelSource;

    private MailboxManager m_mailboxManager;

    @Required
    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

    @Required
    public void setPhoneModelSource(ModelSource<PhoneModel> modelSource) {
        m_modelSource = modelSource;
    }

    @Required
    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }

    @Required
    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    /**
     * @param input - one row of imported data
     * @return CheckRowDataRetVal
     */
    @Override
    protected RowStatus checkRowData(String[] row) {
        String userName = Index.USERNAME.get(row);
        String serialNo = Index.SERIAL_NUMBER.get(row);

        if (isBlank(serialNo) && isBlank(userName)) {
            return RowStatus.FAILURE;
        }

        if (isNotBlank(userName)) {
            // check for a valid user name
            if (!UserValidationUtils.isValidUserName(userName)) {
                return RowStatus.FAILURE;
            }
        }
        return RowStatus.SUCCESS;
    }

    /**
     * Inserts data from a single row
     *
     * @param row each array element represents a single field - see Index
     */
    @Override
    protected void insertRow(String[] row) {
        User user = userFromRow(row);
        Collection<Group> userGroups = null;
        if (user != null) {
            String userGroupName = Index.USER_GROUP.get(row);
            userGroups = m_settingDao.getGroupsByString(CoreContext.USER_GROUP_RESOURCE_ID,
                    userGroupName, true);
        }

        Phone phone = phoneFromRow(row);
        Collection<Group> phoneGroups;
        if (phone != null) {
            String phoneGroupName = Index.PHONE_GROUP.get(row);
            phoneGroups = m_settingDao.getGroupsByString(PhoneContext.GROUP_RESOURCE_ID,
                    phoneGroupName, true);
        } else {
            phoneGroups = null;
        }

        String additionalLineSettings = Index.ADDITIONAL_LINE_SETTINGS.get(row);
        insertData(user, userGroups, phone, phoneGroups, additionalLineSettings);
    }

    /**
     * Updates user properties from row data. Creates new user if one does not exist. USer (newly
     * created or updated) is not save to the database here. That's reponsibility of the called By
     * convention empty String does not overwrite the data.
     *
     * @param row see Index enum
     *
     * @return modified (but not saved used object)
     */
    User userFromRow(String[] row) {
        String userName = Index.USERNAME.get(row);
        if (userName.length() == 0) {
            return null;
        }
        User user = m_coreContext.loadUserByUserName(userName);

        if (user == null) {
            user = m_coreContext.newUser();
            user.setUserName(userName);
        }
        Index.PIN.setProperty(user, row);

        String voicemailPin = Index.VOICEMAIL_PIN.get(row);
        if (isHashed(voicemailPin)) {
            user.setVoicemailPintoken(voicemailPin);
        } else {
            user.setVoicemailPintoken(Md5Encoder.digestEncryptPassword(userName, voicemailPin));
        }

        Index.FIRST_NAME.setProperty(user, row);
        Index.LAST_NAME.setProperty(user, row);
        Index.ALIAS.setProperty(user, row);
        Index.SIP_PASSWORD.setProperty(user, row);
        Index.SALUTATION.setProperty(user, row);
        Index.MANAGER.setProperty(user, row);
        Index.EMPLOYEE_ID.setProperty(user, row);
        Index.IM_ID.setProperty(user, row);
        Index.EMAIL.setProperty(user, row);
        Index.JOB_TITLE.setProperty(user, row);
        Index.JOB_DEPT.setProperty(user, row);
        Index.COMPANY_NAME.setProperty(user, row);
        Index.ASSISTANT_NAME.setProperty(user, row);
        Index.CELL_PHONE_NUMBER.setProperty(user, row);
        Index.HOME_PHONE_NUMBER.setProperty(user, row);
        Index.ASSISTANT_PHONE_NUMBER.setProperty(user, row);
        Index.FAX_NUMBER.setProperty(user, row);
        Index.DID_NUMBER.setProperty(user, row);
        Index.ALTERNATE_EMAIL.setProperty(user, row);
        Index.ALTERNATE_IM_ID.setProperty(user, row);
        Index.LOCATION.setProperty(user, row);
        Index.HOME_STREET.setProperty(user, row);
        Index.HOME_CITY.setProperty(user, row);
        Index.HOME_STATE.setProperty(user, row);
        Index.HOME_COUNTRY.setProperty(user, row);
        Index.HOME_ZIP.setProperty(user, row);
        Index.OFFICE_STREET.setProperty(user, row);
        Index.OFFICE_CITY.setProperty(user, row);
        Index.OFFICE_STATE.setProperty(user, row);
        Index.OFFICE_COUNTRY.setProperty(user, row);
        Index.OFFICE_ZIP.setProperty(user, row);
        Index.OFFICE_MAIL_STOP.setProperty(user, row);
        Index.TWITTER_NAME.setProperty(user, row);
        Index.LINKEDIN_NAME.setProperty(user, row);
        Index.FACEBOOK_NAME.setProperty(user, row);
        Index.XING_NAME.setProperty(user, row);
        Index.ACTIVE_GREETING.setProperty(user, row);
        Index.PRIMARY_EMAIL_NOTIFICATION.setProperty(user, row);
        Index.PRIMARY_EMAIL_FORMAT.setProperty(user, row);
        Index.PRIMARY_EMAIL_ATTACH_AUDIO.setProperty(user, row);
        Index.ALT_EMAIL_NOTIFICATION.setProperty(user, row);
        Index.ALT_EMAIL_FORMAT.setProperty(user, row);
        Index.ALT_EMAIL_ATTACH_AUDIO.setProperty(user, row);
        Index.VOICEMAIL_SERVER.setProperty(user, row);
        Index.EXTERNAL_NUMBER.setProperty(user, row);
        Index.ANONYMOUS_CALLER_ALIAS.setProperty(user, row);

        return user;
    }

    Phone phoneFromRow(String[] row) {
        String serialNo = Index.SERIAL_NUMBER.get(row);
        if (serialNo.length() == 0) {
            return null;
        }

        String modelId = Index.MODEL_ID.get(row).trim();
        PhoneModel model = m_modelSource.getModel(modelId);
        serialNo = model.cleanSerialNumber(Index.SERIAL_NUMBER.get(row));

        Integer phoneId = m_phoneContext.getPhoneIdBySerialNumber(serialNo);
        Phone phone = null;
        if (phoneId != null) {
            phone = m_phoneContext.loadPhone(phoneId);
        } else {
            phone = m_phoneContext.newPhone(model);
            phone.setSerialNumber(serialNo);
        }

        String description = Index.PHONE_DESCRIPTION.get(row);
        if (description.length() > 0) {
            phone.setDescription(description);
        }

        String additionalphoneSettings = Index.ADDITIONAL_PHONE_SETTINGS.get(row);
        if (!StringUtils.isEmpty(additionalphoneSettings)) {
            phone.setAdditionalPhoneSettings(additionalphoneSettings);
        }

        return phone;
    }

    /**
     * Creates user and adds it to user group, creates phone and adds it to phones group; then
     * creates the line for a newly added group on newly added phone.
     *
     * @param user user to add or update
     * @param phone phone to add or update
     * @param userGroup user group to which user will be added
     * @param phoneGroup phone group to which phone will be added
     */
    private void insertData(User user, Collection<Group> userGroups, Phone phone,
            Collection<Group> phoneGroups, String settings) {

        if (user != null) {
            for (Group userGroup : userGroups) {
                user.addGroup(userGroup);
            }
            // Execute the automatic assignments for the user.
            GroupAutoAssign groupAutoAssign = new GroupAutoAssign(m_conferenceBridgeContext, m_coreContext,
                                                                  m_forwardingContext, m_mailboxManager);
            //this method will call coreContext.saveUser
            groupAutoAssign.assignUserData(user);
        }

        if (phoneGroups != null) {
            for (Group phoneGroup : phoneGroups) {
                phone.addGroup(phoneGroup);
            }
        }

        if (phone != null) {
            addLine(phone, user, settings);
            m_phoneContext.storePhone(phone);
        }
    }

    void updateMailbox(User user, boolean newMailbox) {
        if (!m_mailboxManager.isEnabled()) {
            return;
        }
        String userName = user.getUserName();
        if (newMailbox) {
            m_mailboxManager.deleteMailbox(userName);
        }
    }

    Line addLine(Phone phone, User user, String settings) {
        if (user == null) {
            return null;
        }
        for (Line l : phone.getLines()) {
            User candidate = l.getUser();
            if (candidate != null && candidate.equals(user)) {
                // user already on this line
                if (!StringUtils.isEmpty(settings)) {
                    l.setAdditionalLineSettings(settings);
                }
                return l;
            }
        }
        Line line = phone.createLine();
        if (!StringUtils.isEmpty(settings)) {
            line.setAdditionalLineSettings(settings);
        }
        line.setUser(user);
        phone.addLine(line);
        return line;
    }

    @Override
    protected String dataToString(String[] row) {
        List<String> ids = new ArrayList<String>();
        Index[] fields = new Index[] {
            Index.USERNAME, Index.SERIAL_NUMBER
        };
        for (Index f : fields) {
            String val = f.get(row);
            if (isNotBlank(val)) {
                ids.add(val);
            }
        }
        return join(ids, " ");
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }
}
