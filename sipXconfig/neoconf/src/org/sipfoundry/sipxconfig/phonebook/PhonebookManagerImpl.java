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

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.Writer;
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.apache.commons.collections.Closure;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
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
import org.hibernate.Hibernate;
import org.hibernate.HibernateException;
import org.hibernate.classic.Session;
import org.sipfoundry.sipxconfig.bulk.BulkParser;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;

import static org.apache.commons.lang.StringUtils.defaultString;
import static org.sipfoundry.sipxconfig.common.DaoUtils.checkDuplicates;
import static org.sipfoundry.sipxconfig.common.DaoUtils.requireOneOrZero;

public class PhonebookManagerImpl extends SipxHibernateDaoSupport<Phonebook> implements PhonebookManager,
        DaoEventListener {
    private static final Log LOG = LogFactory.getLog(PhonebookManagerImpl.class);

    private static final String NAME = "name";
    private static final String FIELD_ID = "id";
    private static final String FIELD_CONTENT = "content";

    private boolean m_phonebookManagementEnabled;
    private String m_externalUsersDirectory;
    private CoreContext m_coreContext;

    private BulkParser m_csvParser;
    private BulkParser m_vcardParser;
    private VcardWriter m_vcardWriter;
    private String m_vcardEncoding;
    private String m_csvEncoding;

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
        checkDuplicates(getHibernateTemplate(), Phonebook.class, phonebook, NAME, new DuplicatePhonebookName());
        getHibernateTemplate().saveOrUpdate(phonebook);
    }

    class DuplicatePhonebookName extends UserException {
        DuplicatePhonebookName() {
            super("A phonebook already exists with that name.");
        }
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setCsvParser(BulkParser csvParser) {
        m_csvParser = csvParser;
    }

    @Required
    public void setCvsEncoding(String cvsEncoding) {
        m_csvEncoding = cvsEncoding;
    }

    @Required
    public void setVcardParser(BulkParser vcardParser) {
        m_vcardParser = vcardParser;
    }

    @Required
    public void setVcardEncoding(String vcardEncoding) {
        m_vcardEncoding = vcardEncoding;
    }

    /**
     * Where external user lists are kept.
     *
     * This method does not ensure that directory exists.
     */
    public String getExternalUsersDirectory() {
        return m_externalUsersDirectory;
    }

    @Required
    public void setExternalUsersDirectory(String externalUsersDirectory) {
        m_externalUsersDirectory = externalUsersDirectory;
    }

    public Phonebook getPhonebookByName(String name) {
        String query = "phoneBookByName";
        Collection<Phonebook> books = getHibernateTemplate().findByNamedQueryAndNamedParam(query, NAME, name);
        return requireOneOrZero(books, query);
    }

    public Collection<Phonebook> getPhonebooksByUser(User consumer) {
        Collection<Phonebook> books = getHibernateTemplate().findByNamedQueryAndNamedParam("phoneBooksByUser",
                "userId", consumer.getId());
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

        for (PhonebookFileEntry fileEntry : phonebook.getEntries()) {
            entries.put(fileEntry.getNumber(), fileEntry);
        }

        List<PhonebookEntry> finalList = new ArrayList(entries.values());
        Collections.sort(finalList, new PhoneEntryComparator());
        return finalList;
    }

    private void addEntriesFromFile(Map<String, PhonebookEntry> entries, InputStream in, String encoding,
            BulkParser parser) throws IOException {
        Reader fileReader = new InputStreamReader(in, encoding);
        parser.parse(fileReader, new PhonebookEntryMaker(entries));
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
        return m_coreContext.loadUserByUserName(entry.getNumber());
    }

    /*
     * @param queryString A case-insensitive query string
     */
    private List<Document> doSearch(Searcher searcher, String queryString) throws IOException, ParseException {
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

    static class PhoneEntryComparator implements Comparator<PhonebookEntry> {
        public int compare(PhonebookEntry a, PhonebookEntry b) {
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

    static class PhonebookEntryMaker implements Closure {
        private final Map<String, PhonebookEntry> m_entries;

        PhonebookEntryMaker(Map entries) {
            m_entries = entries;
        }

        public void execute(Object input) {
            String[] row = (String[]) input;
            PhonebookEntry entry = new StringArrayPhonebookEntry(row);
            m_entries.put(entry.getNumber(), entry);
        }

    }

    /**
     * public so that it works with Velocity
     */
    public static class StringArrayPhonebookEntry implements PhonebookEntry {
        private final String[] m_row;

        StringArrayPhonebookEntry(String... row) {
            if (row.length < 3) {
                throw new UserException("&msg.invalidPhonebookFormat");
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

        public AddressBookEntry getAddressBookEntry() {
            if (m_row.length < 25) {
                return null;
            }

            AddressBookEntry abe = new AddressBookEntry();
            abe.setJobTitle(m_row[3]);
            abe.setJobDept(m_row[4]);
            abe.setCompanyName(m_row[5]);
            abe.setAssistantName(m_row[6]);
            abe.setCellPhoneNumber(m_row[7]);
            abe.setHomePhoneNumber(m_row[8]);
            abe.setAssistantPhoneNumber(m_row[9]);
            abe.setFaxNumber(m_row[10]);
            abe.setImId(m_row[11]);
            abe.setAlternateImId(m_row[12]);
            abe.setLocation(m_row[13]);

            Address homeAddress = new Address();
            homeAddress.setCity(m_row[14]);
            homeAddress.setCountry(m_row[15]);
            homeAddress.setState(m_row[16]);
            homeAddress.setStreet(m_row[17]);
            homeAddress.setZip(m_row[18]);
            abe.setHomeAddress(homeAddress);

            Address officeAddress = new Address();
            officeAddress.setCity(m_row[19]);
            officeAddress.setCountry(m_row[20]);
            officeAddress.setState(m_row[21]);
            officeAddress.setStreet(m_row[22]);
            officeAddress.setZip(m_row[23]);
            officeAddress.setOfficeDesignation(m_row[24]);
            abe.setOfficeAddress(officeAddress);

            return abe;
        }
    }

    /**
     * public so Velocity doesn't reject object
     */
    public static class UserPhonebookEntry implements PhonebookEntry {
        private final User m_user;

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
            return StringUtils.defaultIfEmpty(m_user.getExtension(true), m_user.getUserName());
        }

        public AddressBookEntry getAddressBookEntry() {
            return m_user.getAddressBookEntry();
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

    public boolean getPhonebookManagementEnabled() {
        return m_phonebookManagementEnabled;
    }

    public void setPhonebookManagementEnabled(boolean phonebookManagementEnabled) {
        m_phonebookManagementEnabled = phonebookManagementEnabled;
    }

    public void setVcardWriter(VcardWriter vcardWriter) {
        m_vcardWriter = vcardWriter;
    }

    public void exportPhonebook(Collection<PhonebookEntry> entries, OutputStream out) throws IOException {
        Writer writer = new OutputStreamWriter(out, m_vcardEncoding);
        for (PhonebookEntry entry : entries) {
            m_vcardWriter.write(writer, entry);
        }
        writer.flush();
    }

    public void addEntriesFromFile(Integer phonebookId, InputStream is) {
        try {
            Phonebook phonebook = getPhonebook(phonebookId);
            addEntries(phonebook, is);
            savePhonebook(phonebook);
        } catch (IOException e) {
            throw new UserException("&msg.phonebookUploadError");
        }
    }

    void addEntries(Phonebook phonebook, InputStream is) throws IOException {
        Map<String, PhonebookEntry> entries = new TreeMap<String, PhonebookEntry>();

        BufferedInputStream in = new BufferedInputStream(is);
        if (isVcard(in)) {
            addEntriesFromFile(entries, in, m_vcardEncoding, m_vcardParser);
        } else {
            addEntriesFromFile(entries, in, m_csvEncoding, m_csvParser);
        }

        List<PhonebookEntry> entriesList = new ArrayList(entries.values());

        for (PhonebookEntry entry : entriesList) {
            PhonebookFileEntry fileEntry = new PhonebookFileEntry();

            fileEntry.setFirstName(entry.getFirstName());
            fileEntry.setLastName(entry.getLastName());
            fileEntry.setNumber(entry.getNumber());
            fileEntry.setAddressBookEntry(entry.getAddressBookEntry());
            fileEntry.setPhonebook(phonebook);

            phonebook.addEntry(fileEntry);
        }
    }

    @SuppressWarnings("deprecation")
    public void removeTableColumns() {
        try {
            String[] queries = {
                "alter table phonebook drop column members_csv_filename",
                "alter table phonebook drop column members_vcard_filename"
            };
            Session currentSession = getHibernateTemplate().getSessionFactory().getCurrentSession();
            Connection connection = currentSession.connection();
            Statement statement = connection.createStatement();
            statement.addBatch(queries[0]);
            statement.addBatch(queries[1]);
            statement.executeBatch();
            statement.close();
            LOG.info("Columns members_csv_filename and members_vcard_filename were removed from phonebook table.");
        } catch (SQLException e) {
            LOG.warn(e.getMessage());
        }

    }

    /**
     * Retrieve the name of file in which phone book entries were kept
     *
     * This method is only used during upgrade: in 4.0 sipXconfig didn't keep list of phonebook
     * entries in DB. It only kept references to external files. Every time phonebook was
     * generated files were parsed. During upgrade to 4.2 files are parsed, entries are stored in
     * DB and later files are discarded.
     *
     * Since Phonebook object does not have fields for storing filenames any more we need to run
     * direct SQL query to retrieve them.
     *
     */
    public Map<Integer, String[]> getPhonebookFilesName() {
        Map<Integer, String[]> names = new TreeMap<Integer, String[]>();
        try {
            String query = "select phonebook_id, members_csv_filename, members_vcard_filename from phonebook;";
            Session currentSession = getHibernateTemplate().getSessionFactory().getCurrentSession();
            List<Object[]> entries = currentSession.createSQLQuery(query).addScalar("phonebook_id",
                    Hibernate.INTEGER).addScalar("members_csv_filename", Hibernate.STRING).addScalar(
                    "members_vcard_filename", Hibernate.STRING).list();
            for (Object[] entry : entries) {
                String[] files = {
                    (String) entry[1], (String) entry[2]
                };
                names.put((Integer) entry[0], files);
            }
            LOG.info("Extracted files names from " + names.size() + " phonebooks.");
        } catch (HibernateException e) {
            LOG.warn(e.getMessage());
        }
        return names;
    }

    /**
     * Trying to determine if the file is vCard file
     */
    boolean isVcard(BufferedInputStream is) {
        try {
            is.mark(100);
            BufferedReader isr = new BufferedReader(new InputStreamReader(is, m_vcardEncoding));
            String line = isr.readLine();
            boolean isVcard = "BEGIN:VCARD".equalsIgnoreCase(line);
            is.reset();
            return isVcard;
        } catch (IOException e) {
            return false;
        }
    }
}
