/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.phonebook;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.request.IUploadFile;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager.PhonebookFormat;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

import static org.apache.commons.lang.StringUtils.isBlank;

public abstract class EditPhonebook extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "phonebook/EditPhonebook";

    @InjectObject("spring:phonebookManager")
    public abstract PhonebookManager getPhonebookManager();

    @InjectObject("spring:settingDao")
    public abstract SettingDao getSettingDao();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist("client")
    public abstract Integer getPhonebookId();

    public abstract void setPhonebookId(Integer phonebookId);

    public abstract Phonebook getPhonebook();

    public abstract void setPhonebook(Phonebook phonebook);

    public abstract String getMemberGroupsString();

    public abstract void setMemberGroupsString(String groups);

    public abstract String getConsumerGroupsString();

    public abstract void setConsumerGroupsString(String groups);

    public abstract IUploadFile getUploadFile();

    public abstract String getGoogleAddress();

    public abstract String getGooglePassword();

    public PhonebookFormat getVcardFormat() {
        return PhonebookFormat.VCARD;
    }

    public PhonebookFormat getCsvFormat() {
        return PhonebookFormat.CSV;
    }

    public void importGoogleAddressBook() {
        String googleAddress = getGoogleAddress();
        if (isBlank(googleAddress)) {
            getValidator().record(new ValidatorException(getMessages().getMessage("msg.emptyGoogleAccount")));
            return;
        }
        String googlePassword = getGooglePassword();
        if (isBlank(googlePassword)) {
            getValidator().record(new ValidatorException(getMessages().getMessage("msg.emptyGooglePassword")));
            return;
        }
        int contacts = getPhonebookManager().addEntriesFromGoogleAccount(getPhonebookId(),
                googleAddress, googlePassword);
        recordSuccessImport(contacts);
    }

    public void importFromFile() {
        if (getUploadFile() != null) {
            int contacts = getPhonebookManager().addEntriesFromFile(getPhonebookId(), getUploadFile().getStream());
            recordSuccessImport(contacts);
        } else {
            getValidator().record(new ValidatorException(getMessages().getMessage("msg.emptyImportFile")));
        }
    }

    private void recordSuccessImport(int contacts) {
        TapestryUtils.recordSuccess(this,
                getMessages().format("msg.vcardImport.success", contacts));
    }

    public void savePhonebook() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        Phonebook phonebook = getPhonebook();

        String members = getMemberGroupsString();
        if (members != null) {
            List<Group> groups = getSettingDao().getGroupsByString(User.GROUP_RESOURCE_ID, members, true);
            phonebook.replaceMembers(new HashSet<Group>(groups));
        }
        String comsumers = getConsumerGroupsString();
        if (comsumers != null) {
            List<Group> groups = getSettingDao().getGroupsByString(User.GROUP_RESOURCE_ID, comsumers, true);
            phonebook.replaceConsumers(new HashSet<Group>(groups));
        }

        getPhonebookManager().savePhonebook(phonebook);
        setPhonebookId(phonebook.getId());
    }

    public void pageBeginRender(PageEvent event) {
        Phonebook phonebook = getPhonebook();
        if (phonebook == null) {
            Integer phonebookId = getPhonebookId();
            if (phonebookId == null) {
                phonebook = new Phonebook();
            } else {
                phonebook = getPhonebookManager().getPhonebook(phonebookId);
            }
            setPhonebook(phonebook);

            Set<Group> members = phonebook.getMembers();
            String membersString = BeanWithGroups.getGroupsAsString(members);
            setMemberGroupsString(membersString);

            Set<Group> consumers = phonebook.getConsumers();
            String consumersString = BeanWithGroups.getGroupsAsString(consumers);
            setConsumerGroupsString(consumersString);
        }
    }
}
