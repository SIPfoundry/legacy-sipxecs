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

import java.io.IOException;
import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.apache.lucene.queryParser.ParseException;
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
    private static final String UNKNOWN = "label.unknown";

    @Bean
    public abstract EvenOdd getRowClass();

    @InjectObject("spring:phonebookManager")
    public abstract PhonebookManager getPhonebookManager();

    public abstract void setQuery(String query);

    public abstract String getQuery();

    public abstract Collection<PhonebookEntry> getPhonebookEntries();

    public abstract void setPhonebookEntries(Collection<PhonebookEntry> entries);

    public abstract PhonebookEntry getPhonebookEntry();

    public abstract void setPhonebookEntry(PhonebookEntry entry);

    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);

        if (getPhonebookEntries() == null) {
            initializeEntries();
        }
    }

    private void initializeEntries() {
        Collection<Phonebook> phonebooks = getPhonebooks();
        Collection<PhonebookEntry> phonebookEntries = getPhonebookManager().getEntries(phonebooks);
        setPhonebookEntries(phonebookEntries);
    }

    /**
     * Gets a comma-separated list of extensions for the user in the current row
     * 
     * @return A String containing a comma-separated list of phone extensions
     */
    public String getExtensionsForCurrentEntry() {
        User user = getUserForEntry(getPhonebookEntry());
        StringBuffer extensionBuffer = new StringBuffer();

        if (user != null) {
            parseExtensionsForUser(user, extensionBuffer);
        } else {
            if (getPhonebookEntry().getNumber().matches(EXTENSION_PATTERN)) {
                extensionBuffer.append(getPhonebookEntry().getNumber());
            }
        }

        if (extensionBuffer.length() == 0) {
            extensionBuffer.append(getMessages().getMessage(UNKNOWN));
        }

        return extensionBuffer.toString();
    }

    /**
     * Gets a comma-separated list of sip id's for the user in the current row
     * 
     * @return A String containing a comman-separated list of sip id's
     */
    public String getSipIdsForCurrentEntry() {
        User user = getUserForEntry(getPhonebookEntry());
        StringBuffer extensionBuffer = new StringBuffer();

        if (user != null) {
            parseSipIdsForUser(user, extensionBuffer);
        } else {
            if (!getPhonebookEntry().getNumber().matches(EXTENSION_PATTERN)) {
                extensionBuffer.append(getPhonebookEntry().getNumber());
            }
        }

        if (extensionBuffer.length() == 0) {
            extensionBuffer.append(getMessages().getMessage(UNKNOWN));
        }

        return extensionBuffer.toString();
    }

    /**
     * Filters the phonebook entries based on the value of getQuery()
     */
    public void search() throws IOException, ParseException {
        if (getQuery() == null) {
            setQuery("");
        }
        setPhonebookEntries(getPhonebookManager().search(getPhonebooks(), getQuery()));
    }

    public void reset() {
        setQuery(StringUtils.EMPTY);
        initializeEntries();
    }

    private Collection<Phonebook> getPhonebooks() {
        User user = getUser();
        return getPhonebookManager().getPhonebooksByUser(user);
    }

    private User getUserForEntry(PhonebookEntry entry) {
        return getCoreContext().loadUserByUserName(entry.getNumber());
    }

    private void parseExtensionsForUser(User user, StringBuffer extensionBuffer) {
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
    }

    private void parseSipIdsForUser(User user, StringBuffer extensionBuffer) {
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
    }
}
