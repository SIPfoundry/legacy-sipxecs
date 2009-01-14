/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.phonebook;

import java.io.File;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class EditPhonebook extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "phonebook/EditPhonebook";

    public abstract PhonebookManager getPhonebookManager();

    public abstract SettingDao getSettingDao();

    public abstract CoreContext getCoreContext();

    public abstract Phonebook getPhonebook();

    public abstract void setPhonebook(Phonebook phonebook);

    public abstract String getMemberGroupsString();

    public abstract void setMemberGroupsString(String groups);

    public abstract String getConsumerGroupsString();

    public abstract void setConsumerGroupsString(String groups);

    public abstract void setPhonebookId(Integer phonebookId);

    public abstract Integer getPhonebookId();

    public void savePhonebook() {
        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils.getValidator(this);

        if (!TapestryUtils.isValid(this)) {
            return;
        }

        Phonebook phonebook = getPhonebook();

        String assetFilename = phonebook.getMembersCsvFilename();
        try {
            getPhonebookManager().checkPhonebookCsvFileFormat(assetFilename);
        } catch (UserException e) {
            validator.record(e.getMessage(), ValidationConstraint.REQUIRED);
            phonebook.setMembersCsvFilename(null);
            return;
        }

        String groupsString = getMemberGroupsString();
        if (groupsString != null) {
            List<Group> groups = getSettingDao().getGroupsByString(User.GROUP_RESOURCE_ID,
                    groupsString, true);
            phonebook.replaceMembers(new HashSet<Group>(groups));
        }
        String comsumers = getConsumerGroupsString();
        if (comsumers != null) {
            List<Group> groups = getSettingDao().getGroupsByString(User.GROUP_RESOURCE_ID,
                    comsumers, true);
            phonebook.replaceConsumers(new HashSet<Group>(groups));
        }

        getPhonebookManager().savePhonebook(phonebook);
        setPhonebookId(phonebook.getId());
    }

    public File getPhonebookCsvFile() {
        String assetFilename = getPhonebook().getMembersCsvFilename();
        if (assetFilename == null) {
            return null;
        }
        return new File(getPhonebookDirectory(), assetFilename);
    }

    public void setPhonebookCsvFile(File phonebook) {
        getPhonebook().setMembersCsvFilename(phonebook.getName());
    }

    public File getPhonebookVcardFile() {
        String assetFilename = getPhonebook().getMembersVcardFilename();
        if (assetFilename == null) {
            return null;
        }
        return new File(getPhonebookDirectory(), assetFilename);
    }

    public void setPhonebookVcardFile(File phonebook) {
        getPhonebook().setMembersVcardFilename(phonebook.getName());
    }

    public File getPhonebookDirectory() {
        File d = new File(getPhonebookManager().getExternalUsersDirectory());
        return d;
    }

    public void pageBeginRender(PageEvent arg0) {
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
