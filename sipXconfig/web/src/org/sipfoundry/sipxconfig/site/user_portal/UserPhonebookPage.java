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
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
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
    private static final String FIELD_ID = "id";
    private static final String FIELD_CONTENT = "content";
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
    public abstract void setPhonebookEntry(PhonebookEntry entry);

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
            extensionBuffer.append(UNKNOWN);
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
            extensionBuffer.append(UNKNOWN);
        }

        return extensionBuffer.toString();
    }

    

    /**
     * Filters the phonebook entries based on the value of getQuery()
     */
    public void search() throws IOException, ParseException {
        RAMDirectory index = new RAMDirectory();
        IndexWriter writer = new IndexWriter(index, new StandardAnalyzer(), true);
        
        Map<String, PhonebookEntry> usersToEntries = new HashMap<String, PhonebookEntry>();
        for (PhonebookEntry entry : getPhonebookEntries()) {
            Document doc = null;
            User user = getUserForEntry(entry);
            if (user != null) {
                doc = documentFromUser(user);
            } else {
                doc = documentFromPhonebookEntry(entry);
            }
            
            usersToEntries.put(doc.get(FIELD_ID), entry);
            writer.addDocument(doc);
            
        }
        
        writer.optimize();
        writer.close();
        
        Searcher searcher = new IndexSearcher(index);
        List<Document> searchResults = doSearch(searcher);
        List<PhonebookEntry> entryResults = new ArrayList<PhonebookEntry>();
        for (Document doc : searchResults) {
            entryResults.add(usersToEntries.get(doc.get(FIELD_ID)));
        }
        
        setPhonebookEntries(entryResults);
    }

    public void reset() {
        setQuery(StringUtils.EMPTY);
        initializeEntries();
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
        doc.add(new Field(FIELD_CONTENT, value, Field.Store.NO, Field.Index.TOKENIZED));
    }
    
    private List<Document> doSearch(Searcher searcher) throws IOException, ParseException {
        Term searchTerm = new Term(FIELD_CONTENT, getQuery());
        Query query = new PrefixQuery(searchTerm);
        Hits hits = searcher.search(query);
        List<Document> docs = new ArrayList<Document>();
        for (int i = 0; i < hits.length(); i++) {
            docs.add(hits.doc(i));
        }
        
        return docs;
    }
}
