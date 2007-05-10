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

import static org.apache.commons.lang.StringUtils.defaultString;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.apache.commons.collections.Closure;
import org.sipfoundry.sipxconfig.bulk.csv.CsvParser;
import org.sipfoundry.sipxconfig.bulk.csv.CsvParserImpl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.setting.Group;

public class PhonebookManagerImpl extends SipxHibernateDaoSupport<Phonebook> implements PhonebookManager, 
        DaoEventListener {
    private String m_externalUsersDirectory;
    private CoreContext m_coreContext;

    public Collection<Phonebook> getPhonebooks() {
        Collection<Phonebook> books = getHibernateTemplate().loadAll(Phonebook.class);
        return books;
    }
    
    public Phonebook getPhonebook(Integer id) {
        Phonebook phonebook = load(Phonebook.class, id);
        return phonebook;
    }
    
    public void deletePhonebooks(Collection<Integer> ids) {
        for (Integer id : ids) {
            deletePhonebook(getPhonebook(id));
        }
    }
    
    public void deletePhonebook(Phonebook phonebook) {
        getHibernateTemplate().delete(phonebook);
    }

    public void savePhonebook(Phonebook phonebook) {
        DaoUtils.checkDuplicates(getHibernateTemplate(), Phonebook.class, phonebook, "name", 
                new DuplicatePhonebookName());
        getHibernateTemplate().saveOrUpdate(phonebook);
    }
    
    class DuplicatePhonebookName extends UserException {
        DuplicatePhonebookName() {
            super("A phonebook already exists with that name.");
        }
    }

    public CoreContext getCoreContext() {
        return m_coreContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    /**
     * Where external user lists are kept. Callers can assume directory exists already 
     */
    public String getExternalUsersDirectory() {
        // HACK: No good place in unit test infrastructure to ensure phonebook directory exists
        // however unit test should be retooled when spring 2.0 gets updated
        return mkdirs(m_externalUsersDirectory);
    }
    
    static String mkdirs(String dir) {
        File d = new File(dir);
        if (!d.exists()) {
            if (!d.mkdirs()) {
                throw new RuntimeException(new IOException("Could not create directory " + d.getAbsolutePath()));
            }
        }
        return dir;
    }    

    public void setExternalUsersDirectory(String externalUsersDirectory) {
        m_externalUsersDirectory = externalUsersDirectory;
    }
    
    public Phonebook getPhonebookByName(String name) {
        String query = "phoneBookByName";
        Collection<Phonebook> books = getHibernateTemplate().findByNamedQueryAndNamedParam(query, 
                "name", name);
        return DaoUtils.requireOneOrZero(books, query);
    }

    public Collection<Phonebook> getPhonebooksByUser(User consumer) {
        Collection<Phonebook> books = getHibernateTemplate().findByNamedQueryAndNamedParam("phoneBooksByUser", 
                "userId", consumer.getId());
        return books;
    }
    
    public Collection<PhonebookEntry> getRows(Collection<Phonebook> phonebooks) {
        if (phonebooks.isEmpty()) {
            return Collections.emptyList();
        }
        Map<String, PhonebookEntry> entries = new TreeMap();
        for (Phonebook phonebook : phonebooks) {
            for (PhonebookEntry entry : getRows(phonebook)) {
                entries.put(entry.getNumber(), entry);
            }
        }
        return entries.values();
    }
    
    public Collection<PhonebookEntry> getRows(Phonebook phonebook) {
        Map<String, PhonebookEntry> entries = new TreeMap();
        Collection<Group> members = phonebook.getMembers();
        if (members != null) {
            for (Group group : members) {
                for (User user : m_coreContext.getGroupMembers(group)) {
                    PhonebookEntry entry = new UserPhonebookEntry(user);
                    entries.put(entry.getNumber(), entry); 
                }
            }
        }
        
        String csvFilename = phonebook.getMembersCsvFilename();
        if (csvFilename != null) {
            File f = new File(new File(m_externalUsersDirectory), csvFilename);
            CsvParser parser = new CsvParserImpl();
            try {
                parser.parse(new FileReader(f), new CsvPhonebookEntryMaker(entries));
            } catch (FileNotFoundException e) {
                throw new RuntimeException(e);
            }
        }
        
        List<PhonebookEntry> finalList = new ArrayList(entries.values());
        Collections.sort(finalList, new PhoneEntryComparator());
        return finalList;
    }
    
    public void reset() {
        for (Phonebook phonebook : getPhonebooks()) {
            deletePhonebook(phonebook);
        }        
    }
    
    static class PhoneEntryComparator implements Comparator {
        public int compare(Object arg0, Object arg1) {
            PhonebookEntry a = (PhonebookEntry) arg0;
            PhonebookEntry b = (PhonebookEntry) arg1;
            int cmp = defaultString(a.getLastName()).compareTo(defaultString(b.getLastName()));
            if (cmp == 0) {
                cmp = defaultString(a.getFirstName()).compareTo(defaultString(b.getFirstName()));
                if (cmp == 0) {
                    cmp = a.getNumber().compareTo(b.getNumber());
                }
            }
            return cmp;
        }        
    }
    
    static class CsvPhonebookEntryMaker implements Closure {
        private Map<String, PhonebookEntry> m_entries; 
        CsvPhonebookEntryMaker(Map entries) {
            m_entries = entries;            
        }
        public void execute(Object input) {
            String[] row = (String[]) input;
            PhonebookEntry entry = new StringArrayPhonebookEntry(row); 
            m_entries.put(entry.getNumber(), entry);
        }
        
    }
    
    /**
     * public for Velocity doesn't reject object
     */
    public static class StringArrayPhonebookEntry implements PhonebookEntry {
        private String[] m_row;
        
        StringArrayPhonebookEntry(String[] row) {
            if (row.length < 3) {
                throw new CsvFileFormatError();
            }
            m_row = row;
        }
        public String getFirstName() {
            return m_row[0];
        }
        
        public String getLastName() {
            return m_row[1];
        }
        
        public String getNumber() {
            return m_row[2];
        }
    }
    
    static class CsvFileFormatError extends UserException {
        public CsvFileFormatError() {
            super("Too few columns. required columns First name, Last name, Number");
        }
    }
    
    /**
     * public so Velocity doesn't reject object
     */
    public static class UserPhonebookEntry implements PhonebookEntry {
        private User m_user;
        UserPhonebookEntry(User user) {
            m_user = user;
        }
        
        public String getFirstName() {
            return m_user.getFirstName();
        }
        
        public String getLastName() {
            return m_user.getLastName();
        }
        
        public String getNumber() {
            return m_user.getUserName();
        }
    }

    public void onDelete(Object entity) {
        if (entity instanceof Group) {
            Group group = (Group) entity;
            getHibernateTemplate().update(group);
            if (User.GROUP_RESOURCE_ID.equals(group.getResource())) {
                for (Phonebook book : getPhonebooks()) {
                    DataCollectionUtil.removeByPrimaryKey(book.getConsumers(), group.getPrimaryKey());
                    DataCollectionUtil.removeByPrimaryKey(book.getMembers(), group.getPrimaryKey());
                    savePhonebook(book);
                }
            }
        }
    }

    public void onSave(Object entity) {
    }
}
