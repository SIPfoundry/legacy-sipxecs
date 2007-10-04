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

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.apache.commons.collections.Closure;
import org.apache.lucene.analysis.standard.StandardAnalyzer;
import org.apache.lucene.document.Document;
import org.apache.lucene.document.Field;
import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.index.Term;
import org.apache.lucene.queryParser.ParseException;
import org.apache.lucene.search.Hits;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.PrefixQuery;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.Searcher;
import org.apache.lucene.store.RAMDirectory;
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

import static org.apache.commons.lang.StringUtils.defaultString;

public class PhonebookManagerImpl extends SipxHibernateDaoSupport<Phonebook> implements
        PhonebookManager, DaoEventListener {
    private static final String NAME = "name";
    private static final String FIELD_ID = "id";
    private static final String FIELD_CONTENT = "content";

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
        DaoUtils.checkDuplicates(getHibernateTemplate(), Phonebook.class, phonebook, NAME,
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
                throw new RuntimeException(new IOException("Could not create directory "
                        + d.getAbsolutePath()));
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
                NAME, name);
        return DaoUtils.requireOneOrZero(books, query);
    }

    public Collection<Phonebook> getPhonebooksByUser(User consumer) {
        Collection<Phonebook> books = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "phoneBooksByUser", "userId", consumer.getId());
        return books;
    }

    public Collection<PhonebookEntry> getEntries(Collection<Phonebook> phonebooks) {
        if (phonebooks.isEmpty()) {
            return Collections.emptyList();
        }
        Map<String, PhonebookEntry> entries = new TreeMap();
        for (Phonebook phonebook : phonebooks) {
            for (PhonebookEntry entry : getEntries(phonebook)) {
                entries.put(entry.getNumber(), entry);
            }
        }
        return entries.values();
    }

    public Collection<PhonebookEntry> getEntries(Phonebook phonebook) {
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

    /**
     * Search the specified phonebooks for all entries that match the given query string. Both the
     * first and last name of each entry are searched using a prefix search (R, Rob, and Robert
     * all match the first name "Robert"). Additionally, for all entries that coorespond to User
     * objects, the aliases of that user object will be searched.
     * 
     * @param queryString The string to search for. Can not be null
     */
    public Collection<PhonebookEntry> search(Collection<Phonebook> phonebooks, String queryString) {
        RAMDirectory index = new RAMDirectory();

        Collection<PhonebookEntry> phonebookEntries = getEntries(phonebooks);

        Map<String, PhonebookEntry> usersToEntries = new HashMap<String, PhonebookEntry>();
        try {
            IndexWriter indexWriter = new IndexWriter(index, new StandardAnalyzer(), true);
            for (PhonebookEntry entry : phonebookEntries) {
                Document doc = null;
                User user = getUserForEntry(entry);
                if (user != null) {
                    doc = documentFromUser(user);
                } else {
                    doc = documentFromPhonebookEntry(entry);
                }

                usersToEntries.put(doc.get(FIELD_ID), entry);
                indexWriter.addDocument(doc);
            }

            indexWriter.optimize();
            indexWriter.close();

            Searcher searcher = new IndexSearcher(index);
            List<Document> searchResults = doSearch(searcher, queryString);
            List<PhonebookEntry> entryResults = new ArrayList<PhonebookEntry>();
            for (Document doc : searchResults) {
                entryResults.add(usersToEntries.get(doc.get(FIELD_ID)));
            }

            return entryResults;
        } catch (IOException ioe) {
            throw new UserException(ioe);
        } catch (ParseException pe) {
            throw new UserException(pe);
        }
    }

    private User getUserForEntry(PhonebookEntry entry) {
        return getCoreContext().loadUserByUserName(entry.getNumber());
    }

    /*
     * @param queryString A case-insensitive query string
     */
    private List<Document> doSearch(Searcher searcher, String queryString) throws IOException,
            ParseException {
        Term searchTerm = new Term(FIELD_CONTENT, queryString.toLowerCase());
        Query query = new PrefixQuery(searchTerm);
        Hits hits = searcher.search(query);
        List<Document> docs = new ArrayList<Document>();
        for (int i = 0; i < hits.length(); i++) {
            docs.add(hits.doc(i));
        }

        return docs;
    }

    private Document documentFromUser(User user) {
        Document doc = new Document();
        addIdToDoc(doc, user.getName());
        addTextFieldToDocument(doc, user.getUserName());
        addTextFieldToDocument(doc, user.getFirstName());
        addTextFieldToDocument(doc, user.getLastName());
        addTextFieldToDocument(doc, user.getAliasesString());

        return doc;
    }

    private Document documentFromPhonebookEntry(PhonebookEntry entry) {
        Document doc = new Document();
        addIdToDoc(doc, entry.getNumber());
        addTextFieldToDocument(doc, entry.getNumber());
        addTextFieldToDocument(doc, entry.getFirstName());
        addTextFieldToDocument(doc, entry.getLastName());

        return doc;
    }

    private void addIdToDoc(Document doc, String id) {
        doc.add(new Field(FIELD_ID, id, Field.Store.YES, Field.Index.UN_TOKENIZED));
    }

    private void addTextFieldToDocument(Document doc, String value) {
        if (value != null) {
            doc.add(new Field(FIELD_CONTENT, value, Field.Store.NO, Field.Index.TOKENIZED));
        }
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
                    DataCollectionUtil.removeByPrimaryKey(book.getConsumers(), group
                            .getPrimaryKey());
                    DataCollectionUtil.removeByPrimaryKey(book.getMembers(), group
                            .getPrimaryKey());
                    savePhonebook(book);
                }
            }
        }
    }

    public void onSave(Object entity) {
    }
}
