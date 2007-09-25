/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import java.util.ArrayList;
import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.bean.EvenOdd;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;

public abstract class UserPhonebookPage extends UserBasePage implements PageBeginRenderListener {

    private static final String SEPARATOR = ", ";
    private static final String EXTENSION_PATTERN = "\\d*";
    private static final String UNKNOWN = "LABEL.UNKNOWN";

    @Bean
    public abstract EvenOdd getRowClass();

    @InjectObject("spring:phonebookManager")
    public abstract PhonebookManager getPhonebookManager();

    public abstract void setQuery(String query);

    public abstract String getQuery();

    public abstract Collection<PhonebookEntry> getPhonebookEntries();

    public abstract void setPhonebookEntries(Collection<PhonebookEntry> entries);

    public abstract PhonebookEntry getPhonebookEntry();

    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);

        if (getPhonebookEntries() == null) {
            initializeEntries();
        }
    }

    private void initializeEntries() {
        User user = getUser();
        Collection<Phonebook> phonebooks = getPhonebookManager().getPhonebooksByUser(user);
        Collection<PhonebookEntry> phonebookEntries = getPhonebookManager().getRows(phonebooks);

        setPhonebookEntries(phonebookEntries);
    }

    /**
     * Gets a comma-separated list of extensions for the user in the current row
     * 
     * @return A String containing a comma-separated list of phone extensions
     */
    public String getExtensionsForCurrentRow() {
        User user = getUserForEntry(getPhonebookEntry());
        StringBuffer extensionBuffer = new StringBuffer();

        if (user.getName().matches(EXTENSION_PATTERN)) {
            extensionBuffer.append(user.getName());
        }

        for (String alias : user.getAliases()) {
            if (alias.matches(EXTENSION_PATTERN)) {
                if (extensionBuffer.length() > 0) {
                    extensionBuffer.append(SEPARATOR);
                }
                extensionBuffer.append(alias);
            }
        }

        if (extensionBuffer.length() == 0) {
            extensionBuffer.append(UNKNOWN);
        }

        return extensionBuffer.toString();
    }

    /**
     * Gets a comma-separated list of sip id's for the user in the current row
     * 
     * @return A String containing a comman-separated list of sip id's
     */
    public String getSipIdsForCurrentRow() {
        User user = getUserForEntry(getPhonebookEntry());
        StringBuffer extensionBuffer = new StringBuffer();

        if (!user.getName().matches(EXTENSION_PATTERN)) {
            extensionBuffer.append(user.getName());
        }

        for (String alias : user.getAliases()) {
            if (!alias.matches(EXTENSION_PATTERN)) {
                if (extensionBuffer.length() > 0) {
                    extensionBuffer.append(SEPARATOR);
                }
                extensionBuffer.append(alias);
            }
        }

        if (extensionBuffer.length() == 0) {
            extensionBuffer.append(UNKNOWN);
        }

        return extensionBuffer.toString();
    }

    /**
     * Filters the phonebook entries based on the value of getQuery()
     */
    public void search() {
        String query = getQuery();
        if (query == null || StringUtils.isEmpty(query)) {
            return;
        }

        Collection<PhonebookEntry> filteredEntries = new ArrayList<PhonebookEntry>();
        Collection<PhonebookEntry> allEntries = getPhonebookEntries();
        for (PhonebookEntry entry : allEntries) {
            User user = getUserForEntry(entry);
            if (query.equals(user.getFirstName()) || query.equals(user.getLastName())
                    || user.getAliases().contains(query) || query.equals(user.getName())) {
                filteredEntries.add(entry);
            }
        }

        setPhonebookEntries(filteredEntries);
    }

    public void reset() {
        setQuery(StringUtils.EMPTY);
        initializeEntries();
    }

    private User getUserForEntry(PhonebookEntry entry) {
        return getCoreContext().loadUserByUserName(entry.getNumber());
    }
}
