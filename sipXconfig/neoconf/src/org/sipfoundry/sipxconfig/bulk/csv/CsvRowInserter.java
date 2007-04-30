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

import org.apache.commons.collections.Closure;
import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.bulk.RowInserter;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

public class CsvRowInserter extends RowInserter<String[]> implements Closure {
    private CoreContext m_coreContext;

    private PhoneContext m_phoneContext;

    private ModelSource<PhoneModel> m_modelSource;

    private MailboxManager m_mailboxManager;

    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

    public void setPhoneModelSource(ModelSource<PhoneModel> modelSource) {
        m_modelSource = modelSource;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    protected boolean checkRowData(String[] row) {
        String userName = Index.USERNAME.get(row);
        String serialNo = Index.SERIAL_NUMBER.get(row);
        return StringUtils.isNotBlank(serialNo) && StringUtils.isNotBlank(userName);
    }

    /**
     * Inserts data from a single row
     * 
     * @param row each array element represents a single field - see Index
     */
    protected void insertRow(String[] row) {
        User user = userFromRow(row);
        Phone phone = phoneFromRow(row);

        String phoneGroupName = Index.PHONE_GROUP.get(row);
        Group phoneGroup = null;
        if (StringUtils.isNotBlank(phoneGroupName)) {
            phoneGroup = m_phoneContext.getGroupByName(phoneGroupName, true);
        }

        String userGroupName = Index.USER_GROUP.get(row);
        Group userGroup = null;
        if (StringUtils.isNotBlank(userGroupName)) {
            userGroup = m_coreContext.getGroupByName(userGroupName, true);
        }

        MailboxPreferences mboxPrefs = mailboxPreferencesFromRow(row);

        insertData(user, userGroup, phone, phoneGroup, mboxPrefs);
    }

    MailboxPreferences mailboxPreferencesFromRow(String[] row) {
        String emailAddress = Index.EMAIL.get(row);
        if (!m_mailboxManager.isEnabled() || StringUtils.isBlank(emailAddress)) {
            return null;
        }
        String userId = Index.USERNAME.get(row);
        Mailbox mailbox = m_mailboxManager.getMailbox(userId);
        MailboxPreferences mboxPrefs = m_mailboxManager.loadMailboxPreferences(mailbox);
        mboxPrefs.setEmailAddress(emailAddress);
        return mboxPrefs;
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
        User user = m_coreContext.loadUserByUserName(userName);

        if (user == null) {
            user = new User();
            user.setUserName(userName);
        }

        String pin = Index.PIN.get(row);
        if (pin.length() > 0) {
            user.setPin(pin, m_coreContext.getAuthorizationRealm());
        }

        Index.FIRST_NAME.setProperty(user, row);
        Index.LAST_NAME.setProperty(user, row);
        Index.ALIAS.setProperty(user, row);
        Index.SIP_PASSWORD.setProperty(user, row);

        return user;
    }

    Phone phoneFromRow(String[] row) {
        String modelId = Index.MODEL_ID.get(row).trim();
        PhoneModel model = m_modelSource.getModel(modelId);
        String serialNo = model.cleanSerialNumber(Index.SERIAL_NUMBER.get(row));

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

        return phone;
    }

    /**
     * Creates user and adds it to user group, creates phone and adds it to phones group; then
     * creates the line for a newly added group on newly added phone.
     * 
     * @param user user to add or update
     * @param userGroup user group to which user will be added
     * @param phone phone to add or update
     * @param phoneGroup phone group to which phone will be added
     */
    private void insertData(User user, Group userGroup, Phone phone, Group phoneGroup,
            MailboxPreferences mboxPrefs) {
        if (userGroup != null) {
            user.addGroup(userGroup);
        }
        m_coreContext.saveUser(user);

        if (phoneGroup != null) {
            phone.addGroup(phoneGroup);
        }

        addLine(phone, user);

        m_phoneContext.storePhone(phone);

        if (mboxPrefs != null) {
            Mailbox mailbox = m_mailboxManager.getMailbox(user.getUserName());
            m_mailboxManager.saveMailboxPreferences(mailbox, mboxPrefs);
        }
    }
    
    Line addLine(Phone phone, User user) {
        for (Line l : phone.getLines()) {
            User candidate = l.getUser();
            if (candidate != null && candidate.equals(user)) {
                // user already on this line
                return l;
            }
        }
        
        Line line = phone.createLine();
        line.setUser(user);
        phone.addLine(line);
        return line;
    }

    protected String dataToString(String[] row) {
        if (row.length > 0 && StringUtils.isNotBlank(row[0])) {
            return row[0];
        }
        return ArrayUtils.toString(row);
    }
}
