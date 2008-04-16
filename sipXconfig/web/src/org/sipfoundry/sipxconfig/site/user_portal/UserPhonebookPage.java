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
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.Messages;
import org.apache.lucene.queryParser.ParseException;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;

public abstract class UserPhonebookPage extends UserBasePage implements PageBeginRenderListener {
    @InjectObject("spring:phonebookManager")
    public abstract PhonebookManager getPhonebookManager();

    @Persist
    public abstract void setQuery(String query);

    public abstract String getQuery();

    public abstract Collection<PhonebookEntry> getPhonebookEntries();

    public abstract void setPhonebookEntries(Collection<PhonebookEntry> entries);

    /**
     * Gets a comma-separated list of extensions for the user in the current row
     * 
     * @return A String containing a comma-separated list of phone extensions
     */
    public abstract String getExtensionsForCurrentEntry();

    public abstract void setExtensionsForCurrentEntry(String value);

    /**
     * Gets a comma-separated list of sip id's for the user in the current row
     * 
     * @return A String containing a comma-separated list of sip id's
     */
    public abstract String getSipIdsForCurrentEntry();

    public abstract void setSipIdsForCurrentEntry(String value);

    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);

        if (getPhonebookEntries() == null) {
            initializeEntries();
        }
    }

    private void initializeEntries() {
        String query = getQuery();
        Collection<Phonebook> phonebooks = getPhonebooks();
        Collection<PhonebookEntry> entries = null;
        if (StringUtils.isEmpty(query)) {
            entries = getPhonebookManager().getEntries(phonebooks);
        } else {
            entries = getPhonebookManager().search(phonebooks, query);
        }
        setPhonebookEntries(entries);
    }

    /**
     * Filters the phonebook entries based on the value of getQuery()
     */
    public void search() throws IOException, ParseException {
        setPhonebookEntries(null);
    }

    public void reset() {
        setQuery(StringUtils.EMPTY);
        setPhonebookEntries(null);
    }

    private Collection<Phonebook> getPhonebooks() {
        User user = getUser();
        return getPhonebookManager().getPhonebooksByUser(user);
    }

    private User getUserForEntry(PhonebookEntry entry) {
        return getCoreContext().loadUserByUserName(entry.getNumber());
    }

    /**
     * Called whenever new row is about to displayed. Sorts entries into extensions (that look
     * like phone numbers) and sipIds (that look like SIP usernames)
     * 
     * @param entry phone book entry
     */
    public void setPhonebookEntry(PhonebookEntry entry) {
        User user = getUserForEntry(entry);
        AliasSorter aliasSorter = new AliasSorter(user, entry);
        Messages messages = getMessages();
        setExtensionsForCurrentEntry(aliasSorter.getExtensions(messages));
        setSipIdsForCurrentEntry(aliasSorter.getSipIds(messages));
    }

    static class AliasSorter {
        private static final Pattern EXTENSION_PATTERN = Pattern.compile("\\d*");

        private List<String> m_sipIds = new ArrayList<String>();
        private List<String> m_extensions = new ArrayList<String>();

        public AliasSorter(User user, PhonebookEntry entry) {
            if (user == null) {
                addAlias(entry.getNumber());

            } else {
                addAlias(user.getName());
                for (String alias : user.getAliases()) {
                    addAlias(alias);
                }
            }
        }

        public String getSipIds(Messages messages) {
            return asString(m_sipIds, messages);
        }

        public String getExtensions(Messages messages) {
            return asString(m_extensions, messages);
        }

        private void addAlias(String alias) {
            Matcher m = EXTENSION_PATTERN.matcher(alias);
            List<String> l = m.matches() ? m_extensions : m_sipIds;
            l.add(alias);
        }

        private String asString(List<String> aliases, Messages messages) {
            if (aliases.isEmpty()) {
                return messages.getMessage("label.unknown");
            }

            return StringUtils.join(aliases, ", ");
        }
    }
}
