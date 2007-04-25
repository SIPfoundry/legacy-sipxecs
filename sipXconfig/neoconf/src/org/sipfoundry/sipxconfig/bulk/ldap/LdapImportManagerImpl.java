/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import javax.naming.NameClassPair;
import javax.naming.NamingException;
import javax.naming.directory.Attributes;
import javax.naming.directory.SearchControls;
import javax.naming.directory.SearchResult;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.bulk.UserPreview;
import org.sipfoundry.sipxconfig.bulk.csv.CsvWriter;
import org.sipfoundry.sipxconfig.bulk.csv.Index;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;
import org.springframework.ldap.CollectingNameClassPairCallbackHandler;
import org.springframework.ldap.LdapTemplate;
import org.springframework.ldap.NameClassPairMapper;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class LdapImportManagerImpl extends HibernateDaoSupport implements LdapImportManager {
    public static final Log LOG = LogFactory.getLog(LdapImportManager.class);

    private LdapTemplate m_ldapTemplate;

    private LdapManager m_ldapManager;

    private LdapRowInserter m_rowInserter;

    private int m_previewSize;

    public void insert() {
        try {
            Iterator<SearchResult> result = search(0).iterator();
            m_rowInserter.beforeInserting();
            while (result.hasNext()) {
                SearchResult searchResult = result.next();
                m_rowInserter.execute(searchResult);
            }
            m_rowInserter.afterInserting();

        } catch (NamingException e) {
            LOG.error("Retrieving users list from LDAP server", e);
        }
    }

    public List<UserPreview> getExample() {
        try {
            ArrayList<UserPreview> example = new ArrayList<UserPreview>(m_previewSize);
            Iterator<SearchResult> result = search(m_previewSize).iterator();
            while (result.hasNext()) {
                SearchResult searchResult = result.next();
                UserPreview preview = getUserPreview(searchResult);
                example.add(preview);
            }
            return example;
        } catch (NamingException e) {
            throw new UserException(e.getMessage());
        }
    }
    
    private UserPreview getUserPreview(SearchResult searchResult) throws NamingException {
        User user = new User();
        Attributes attrs = searchResult.getAttributes();
        m_rowInserter.setUserProperties(user, attrs);
        List<String> groupNames = new ArrayList<String>(m_rowInserter
                .getGroupNames(searchResult));
        MailboxPreferences preferences = m_rowInserter.getMailboxPreferences(attrs);
        UserPreview preview = new UserPreview(user, groupNames, preferences);                
        return preview; 
    }

    public void dumpExample(Writer out) {
        try {
            CsvWriter writer = new CsvWriter(out);
            String[] allNames = Index.getAllNames();
            writer.write(allNames, false);
            
            Iterator<SearchResult> result = search(0).iterator();
            while (result.hasNext()) {
                SearchResult searchResult = result.next();
                UserPreview preview = getUserPreview(searchResult);
                String groupNamesString = StringUtils.join(preview.getGroupNames().iterator(), ", ");
                String[] row = new String[allNames.length];
                Index.USERNAME.set(row, preview.getUser().getUserName());
                Index.FIRST_NAME.set(row, preview.getUser().getFirstName());
                Index.LAST_NAME.set(row, preview.getUser().getLastName());
                Index.ALIAS.set(row, preview.getUser().getAliasesString());
                Index.SIP_PASSWORD.set(row, preview.getUser().getSipPassword());
                Index.EMAIL.set(row, preview.getMailboxPreferences().getEmailAddress());
                Index.USER_GROUP.set(row, groupNamesString);                
                
                writer.write(row, true);
            }
        } catch (NamingException e) {
            throw new UserException(e.getMessage());
        } catch (IOException e) {
            throw new UserException(e.getMessage());
        }
    }

    public void setLdapTemplate(LdapTemplate ldapTemplate) {
        m_ldapTemplate = ldapTemplate;
    }

    public void setRowInserter(LdapRowInserter rowInserter) {
        m_rowInserter = rowInserter;
    }

    public void setLdapManager(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
    }

    public void setPreviewSize(int previewSize) {
        m_previewSize = previewSize;
    }

    private List<SearchResult> search(long limit) throws NamingException {
        SearchControls sc = new SearchControls();
        sc.setCountLimit(limit);
        sc.setSearchScope(SearchControls.SUBTREE_SCOPE);

        AttrMap attrMap = m_ldapManager.getAttrMap();
        if (!attrMap.verified()) {
            m_ldapManager.verify(m_ldapManager.getConnectionParams(), attrMap);
        }

        sc.setReturningAttributes(attrMap.getLdapAttributesArray());

        String base = attrMap.getSearchBase();
        String filter = attrMap.getSearchFilter();

        // FIXME: this is a potential threading problem - we cannot have one template shared
        // if we are changing the connection params for each insert operation
        m_ldapManager.getConnectionParams().applyToTemplate(m_ldapTemplate);
        
        m_rowInserter.setAttrMap(attrMap);
        CollectingNameClassPairCallbackHandler handler = new NameClassPassThru();
        m_ldapTemplate.search(base, filter, sc, handler);
        List<SearchResult> result = handler.getList();
        return result;
    }
    
    class NameClassPassThru extends CollectingNameClassPairCallbackHandler implements NameClassPairMapper {
        public Object mapFromNameClassPair(NameClassPair nameClassPair) throws NamingException {
            return (SearchResult) nameClassPair;
        }

        @Override
        public Object getObjectFromNameClassPair(NameClassPair nameClassPair) {
            try {
                return mapFromNameClassPair(nameClassPair);
            } catch (NamingException e) {
                throw m_ldapTemplate.getExceptionTranslator().translate(e);
            }
        }   
    }

}
