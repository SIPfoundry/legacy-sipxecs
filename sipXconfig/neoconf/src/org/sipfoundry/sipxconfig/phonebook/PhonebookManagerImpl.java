/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phonebook;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
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
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;

import static java.util.Arrays.asList;
import static java.util.Collections.addAll;

import com.glaforge.i18n.io.CharsetToolkit;
import org.apache.commons.collections.Closure;
import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.builder.CompareToBuilder;
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

import static org.apache.commons.lang.StringUtils.join;
import static org.sipfoundry.sipxconfig.common.DaoUtils.checkDuplicates;
import static org.sipfoundry.sipxconfig.common.DaoUtils.requireOneOrZero;

public class PhonebookManagerImpl extends SipxHibernateDaoSupport<Phonebook> implements PhonebookManager,
        DaoEventListener {
    private static final Log LOG = LogFactory.getLog(PhonebookManagerImpl.class);

    private static final String NAME = "name";
    private static final String FIELD_ID = "id";
    private static final String FIELD_CONTENT = "content";
    private static final String PARAM_USER_ID = "userId";

    private boolean m_phonebookManagementEnabled;
    private String m_externalUsersDirectory;
    private CoreContext m_coreContext;

    private BulkParser m_csvParser;
    private BulkParser m_vcardParser;
    private VcardWriter m_vcardWriter;
    private String m_vcardEncoding;

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

    public PhonebookEntry getPhonebookEntry(Integer id) {
        return (PhonebookEntry) getHibernateTemplate().load(PhonebookEntry.class, id);
    }

    public void savePhonebookEntry(PhonebookEntry entry) {
        getHibernateTemplate().saveOrUpdate(entry);
    }

    public void deletePhonebookEntry(PhonebookEntry entry) {
        getHibernateTemplate().delete(entry);
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
                PARAM_USER_ID, consumer.getId());
        return books;
    }

    public Collection<Phonebook> getAllPhonebooksByUser(User consumer) {

        Collection<Phonebook> phonebooks = getPhonebooksByUser(consumer);
        Phonebook privatePhonebook = getPrivatePhonebook(consumer);
        if (privatePhonebook != null) {
            addAll(phonebooks, privatePhonebook);
        }
        return phonebooks;
    }

    public Phonebook getPrivatePhonebook(User user) {
        String query = "privatePhoneBookByUser";
        List<Phonebook> privateBooks = getHibernateTemplate().findByNamedQueryAndNamedParam(query, PARAM_USER_ID,
                user.getId());
        return requireOneOrZero(privateBooks, query);
    }

    public Phonebook getPrivatePhonebookCreateIfRequired(User user) {
        Phonebook phonebook = getPrivatePhonebook(user);
        if (null == phonebook) {
            phonebook = new Phonebook();
            phonebook.setName("privatePhonebook_" + user.getUserName());
            phonebook.setUser(user);
            savePhonebook(phonebook);
        }

        return phonebook;
    }

    public Collection<PhonebookEntry> getEntries(Collection<Phonebook> phonebooks, User user) {
        Map<String, PhonebookEntry> entries = new TreeMap();
        if (!phonebooks.isEmpty()) {
            for (Phonebook phonebook : phonebooks) {
                for (PhonebookEntry entry : getEntries(phonebook)) {
                    entries.put(getEntryKey(entry), entry);
                }
            }
        }

        return entries.values();
    }

    public PagedPhonebook getPagedPhonebook(Collection<Phonebook> phonebook, User user, String startRow,
            String endRow, String queryString) {
        Collection<PhonebookEntry> entries = convertPhonebookEntries(getEntries(phonebook, user));

        // add private phonebook
        Phonebook privatePhonebook = getPrivatePhonebook(user);
        if (privatePhonebook != null) {
            entries.addAll(privatePhonebook.getEntries());
        }

        int totalSize = entries.size();
        if (!StringUtils.isEmpty(queryString) && !queryString.equals("null")) {
            CollectionUtils.filter(entries, new PhonebookEntryPredicate(queryString));
        }

        Collections.sort(new LinkedList(entries), new PhoneEntryComparator());
        return new PagedPhonebook(entries, totalSize, startRow, endRow);
    }

    public Collection<PhonebookEntry> getEntries(Phonebook phonebook) {
        Map<String, PhonebookEntry> entries = new TreeMap();
        Collection<Group> members = phonebook.getMembers();
        if (members != null) {
            for (Group group : members) {
                for (User user : m_coreContext.getGroupMembers(group)) {
                    PhonebookEntry entry = new UserPhonebookEntry(user);
                    entries.put(getEntryKey(entry), entry);
                }
            }
        }

        for (PhonebookEntry fileEntry : phonebook.getEntries()) {
            entries.put(getEntryKey(fileEntry), fileEntry);
        }

        List<PhonebookEntry> finalList = new ArrayList(entries.values());
        Collections.sort(finalList, new PhoneEntryComparator());
        return finalList;
    }

    private void addEntriesFromFile(Map<String, PhonebookEntry> entries, InputStream in, String encoding,
            BulkParser parser, boolean extractHeader) throws IOException {
        Reader fileReader = new InputStreamReader(in, encoding);
        parser.parse(fileReader, new PhonebookEntryMaker(entries, extractHeader));
    }

    /**
     * Search the specified phonebooks for all entries that match the given query string. Both the
     * first and last name of each entry are searched using a prefix search (R, Rob, and Robert
     * all match the first name "Robert"). Additionally, for all entries that coorespond to User
     * objects, the aliases of that user object will be searched.
     *
     * @param queryString The string to search for. Can not be null
     */
    public Collection<PhonebookEntry> search(Collection<Phonebook> phonebooks, String queryString, User portalUser) {
        RAMDirectory index = new RAMDirectory();
        Collection<PhonebookEntry> phonebookEntries = getEntries(phonebooks, portalUser);

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
        addIdToDoc(doc, String.valueOf(entry.getId()));
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
            CompareToBuilder compare = new CompareToBuilder();
            compare.append(a.getLastName(), b.getLastName());
            compare.append(a.getFirstName(), b.getFirstName());
            compare.append(a.getNumber(), b.getNumber());
            return compare.toComparison();
        }
    }

    static class PhonebookEntryPredicate implements Predicate {

        private final String m_queryString;

        public PhonebookEntryPredicate(String queryString) {
            m_queryString = queryString;
        }

        @Override
        public boolean evaluate(Object phoneEntry) {
            if (phoneEntry instanceof PhonebookEntry) {
                PhonebookEntry entry = (PhonebookEntry) phoneEntry;
                if (StringUtils.containsIgnoreCase(entry.getFirstName(), m_queryString)
                        || StringUtils.containsIgnoreCase(entry.getLastName(), m_queryString)
                        || StringUtils.containsIgnoreCase(entry.getNumber(), m_queryString)
                        || (entry.getAddressBookEntry() != null && StringUtils.containsIgnoreCase(entry
                                .getAddressBookEntry().getEmailAddress(), m_queryString))) {
                    return true;
                }
            }
            return false;
        }

    }

    static class PhonebookEntryMaker implements Closure {
        private final Map<String, PhonebookEntry> m_entries;
        private PhonebookFileEntryHelper m_header = new InternalPhonebookVcardHeader();
        private boolean m_extractHeader;

        PhonebookEntryMaker(Map entries, boolean extractHeader) {
            m_entries = entries;
            m_extractHeader = extractHeader;
        }

        public void execute(Object input) {
            if (m_extractHeader) {
                Map<String, Integer> header = extractHeader(input);
                m_header = findHeaderType(header);
                m_extractHeader = false;
            } else {
                String[] row = (String[]) input;
                PhonebookEntry entry = new StringArrayPhonebookEntry(m_header, row);
                m_entries.put(getEntryKey(entry), entry);
            }
        }

        /**
         * Attempt to guess is this is Outlook of GMail generated CSV file.
         *
         * It's based on looking for specific words in header line ('yomi' for GMail and 'tty/tdd
         * phone' for Outlook. Would be nice to have a better method for that.
         *
         */
        private PhonebookFileEntryHelper findHeaderType(Map<String, Integer> header) {
            if (header == null) {
                return new InternalPhonebookCsvHeader();
            }

            // searching for a "yomi" word in the header to see if it is a gmail header
            // or searching for a "tty/tdd phone" word in the header to see if it is an
            // outlook header
            Set<String> keySet = header.keySet();
            for (String key : keySet) {
                if (key.contains("\n")) {
                    throw new InvalidPhonebookFormat();
                }
                if (key.toLowerCase().contains("yomi")) {
                    return new GmailPhonebookCsvHeader(header);
                }
                if (key.toLowerCase().contains("tty/tdd phone")) {
                    return new OutlookPhonebookCsvHeader(header);
                }
            }
            return new InternalPhonebookCsvHeader();
        }

        private Map<String, Integer> extractHeader(Object input) {
            String[] row = (String[]) input;
            Map<String, Integer> header = new HashMap<String, Integer>();
            for (int i = 0; i < row.length; i++) {
                header.put(row[i], i);
            }
            return header;
        }
    }

    /**
     * public so that it works with Velocity
     */
    public static class StringArrayPhonebookEntry extends PhonebookEntry {
        private final String[] m_row;
        private final PhonebookFileEntryHelper m_helper;

        StringArrayPhonebookEntry(String... row) {
            this(new InternalPhonebookCsvHeader(), row);
        }

        StringArrayPhonebookEntry(PhonebookFileEntryHelper helper, String... row) {
            if (row.length < 3) {
                throw new InvalidPhonebookFormat();
            }
            m_row = row;
            m_helper = helper;
        }

        @Override
        public String getFirstName() {
            return m_helper.getFirstName(m_row);
        }

        @Override
        public String getLastName() {
            return m_helper.getLastName(m_row);
        }

        @Override
        public String getNumber() {
            return m_helper.getNumber(m_row);
        }

        @Override
        public AddressBookEntry getAddressBookEntry() {
            return m_helper.getAddressBookEntry(m_row);
        }
    }

    private static class InvalidPhonebookFormat extends UserException {
        public InvalidPhonebookFormat() {
            super("&msg.invalidPhonebookFormat");
        }
    }

    /**
     * public so Velocity doesn't reject object
     */
    public static class UserPhonebookEntry extends PhonebookEntry {
        private final User m_user;

        UserPhonebookEntry(User user) {
            m_user = user;
        }

        @Override
        public String getFirstName() {
            return m_user.getFirstName();
        }

        @Override
        public String getLastName() {
            return m_user.getLastName();
        }

        @Override
        public String getNumber() {
            return StringUtils.defaultIfEmpty(m_user.getExtension(true), m_user.getUserName());
        }

        @Override
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
        } else if (entity instanceof User) {
            User user = (User) entity;
            Phonebook privatePhonebook = getPrivatePhonebook(user);
            if (privatePhonebook != null) {
                deletePhonebook(privatePhonebook);
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

    @Override
    public int addEntriesFromFile(Integer phonebookId, InputStream is) {
        try {
            Phonebook phonebook = getPhonebook(phonebookId);
            int count = addEntries(phonebook, is);
            savePhonebook(phonebook);
            return count;
        } catch (IOException e) {
            throw new UserException("&msg.phonebookUploadError");
        }
    }

    @Override
    public int addEntriesFromGmailAccount(Integer phonebookId, String account, String password) {
        Phonebook phonebook = getPhonebook(phonebookId);
        GoogleImporter googleImporter = new GoogleImporter(account, password);
        int count = googleImporter.addEntries(phonebook);
        savePhonebook(phonebook);
        return count;
    }

    int addEntries(Phonebook phonebook, InputStream is) throws IOException {
        Map<String, PhonebookEntry> entries = new TreeMap<String, PhonebookEntry>();

        BufferedInputStream in = new BufferedInputStream(is);
        String encoding = getEncoding(in);

        if (isVcard(in, encoding)) {
            addEntriesFromFile(entries, in, encoding, m_vcardParser, false);
        } else {
            addEntriesFromFile(entries, in, encoding, m_csvParser, true);
        }

        List<PhonebookEntry> entriesList = new ArrayList(entries.values());
        for (PhonebookEntry entry : entriesList) {
            PhonebookEntry fileEntry = new PhonebookEntry();

            fileEntry.setFirstName(entry.getFirstName());
            fileEntry.setLastName(entry.getLastName());
            fileEntry.setNumber(entry.getNumber());
            fileEntry.setAddressBookEntry(entry.getAddressBookEntry());
            fileEntry.setPhonebook(phonebook);

            phonebook.addEntry(fileEntry);
        }
        return entriesList.size();
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
    boolean isVcard(BufferedInputStream is, String encoding) {
        final String vcardSignature = "BEGIN:VCARD";
        try {
            // keep buffer smaller than the readlimit: trying to ensure that we can reset the
            // stream
            BufferedReader isr = new BufferedReader(new InputStreamReader(is, encoding), vcardSignature.length() * 2);
            is.mark(vcardSignature.length() * 10);
            String line;
            do {
                line = isr.readLine();
            } while (StringUtils.isBlank(line));
            boolean isVcard = vcardSignature.equalsIgnoreCase(line);
            is.reset();
            return isVcard;
        } catch (IOException e) {
            return false;
        }
    }

    private static String getEntryKey(PhonebookEntry entry) {
        return join(asList(entry.getNumber(), entry.getFirstName(), entry.getLastName()), '_');
    }

    public String getEncoding(InputStream is) throws IOException {
        byte[] buffer = new byte[4096];
        is.mark(0);
        is.read(buffer);
        is.reset();

        File tempFile = File.createTempFile("PhonebookFileEntryTemp", null);
        FileOutputStream out = new FileOutputStream(tempFile);
        out.write(buffer);
        out.flush();
        out.close();

        String encoding = CharsetToolkit.guessEncoding(tempFile, buffer.length).displayName();
        tempFile.delete();

        return encoding;
    }

    private Collection<PhonebookEntry> convertPhonebookEntries(Collection<PhonebookEntry> entriesList) {
        Collection<PhonebookEntry> fileEntries = new ArrayList<PhonebookEntry>();
        for (PhonebookEntry entry : entriesList) {
            PhonebookEntry fileEntry = new PhonebookEntry();
            fileEntry.setFirstName(entry.getFirstName());
            fileEntry.setLastName(entry.getLastName());
            fileEntry.setNumber(entry.getNumber());
            fileEntry.setAddressBookEntry(entry.getAddressBookEntry());
            fileEntry.setPhonebook(null);
            fileEntries.add(fileEntry);
        }

        return fileEntries;
    }
}
