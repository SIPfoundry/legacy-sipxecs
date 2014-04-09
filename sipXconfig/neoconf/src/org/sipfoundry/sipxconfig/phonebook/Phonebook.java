/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phonebook;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Set;
import java.util.TreeSet;

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeType;
import org.sipfoundry.sipxconfig.systemaudit.SystemAuditable;

public class Phonebook extends BeanWithId implements NamedObject, SystemAuditable {
    private String m_name;
    private String m_description;
    private boolean m_showOnPhone = true;
    private Set<Group> m_members = new TreeSet<Group>();
    private Set<Group> m_consumers = new TreeSet<Group>();
    private final Set<Group> m_previousConsumers = new TreeSet<Group>();
    private Collection<PhonebookEntry> m_entries = new ArrayList<PhonebookEntry>();
    private User m_user;

    public Set<Group> getMembers() {
        return m_members;
    }

    public void setMembers(Set<Group> members) {
        m_members = members;
    }

    public void replaceMembers(Collection<Group> groups) {
        m_members.clear();
        m_members.addAll(groups);
    }

    public void replaceConsumers(Collection<Group> groups) {
        m_previousConsumers.addAll(m_consumers);
        m_consumers.clear();
        m_consumers.addAll(groups);
    }

    @Override
    public String getName() {
        return m_name;
    }

    @Override
    public void setName(String name) {
        m_name = name;
    }

    public Set<Group> getConsumers() {
        return m_consumers;
    }

    public void setConsumers(Set<Group> consumers) {
        m_consumers = consumers;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public boolean getShowOnPhone() {
        return m_showOnPhone;
    }

    public void setShowOnPhone(boolean show) {
        m_showOnPhone = show;
    }

    public Collection<PhonebookEntry> getEntries() {
        return m_entries;
    }

    public void setEntries(Collection<PhonebookEntry> entries) {
        m_entries = entries;
    }

    public void addEntries(Collection<PhonebookEntry> entries) {
        for (PhonebookEntry entry : entries) {
            addEntry(entry);
        }
    }

    public void addEntry(PhonebookEntry entry) {
        entry.setPhonebook(this);
        m_entries.add(entry);
    }

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public Set<Group> getPreviousConsumers() {
        return m_previousConsumers;
    }

    @Override
    public String getEntityIdentifier() {
        return getName();
    }

    @Override
    public ConfigChangeType getConfigChangeType() {
        return ConfigChangeType.PHONE_BOOK;
    }
}
