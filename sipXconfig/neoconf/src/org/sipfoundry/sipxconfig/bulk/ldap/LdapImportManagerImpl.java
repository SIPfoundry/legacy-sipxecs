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
import java.util.Iterator;
import java.util.List;

import javax.naming.NameClassPair;
import javax.naming.NamingException;
import javax.naming.directory.SearchControls;

import org.apache.commons.collections.Closure;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.bulk.UserPreview;
import org.sipfoundry.sipxconfig.bulk.csv.CsvWriter;
import org.sipfoundry.sipxconfig.bulk.csv.Index;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.dao.DataAccessException;
import org.springframework.ldap.CollectingNameClassPairCallbackHandler;
import org.springframework.ldap.LdapTemplate;
import org.springframework.ldap.NameClassPairCallbackHandler;
import org.springframework.ldap.NameClassPairMapper;
import org.springframework.ldap.SearchLimitExceededException;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class LdapImportManagerImpl extends HibernateDaoSupport implements LdapImportManager {
    public static final Log LOG = LogFactory.getLog(LdapImportManager.class);
    private LdapTemplateFactory m_templateFactory;
    private LdapManager m_ldapManager;
    private LdapRowInserter m_rowInserter;
    private UserMapper m_userMapper;
    private int m_previewSize;

    public void setUserMapper(UserMapper userMapper) {
        m_userMapper = userMapper;
    }

    public void insert() {
        m_rowInserter.setAttrMap(m_ldapManager.getAttrMap());
        m_rowInserter.beforeInserting();
        NameClassPairCallbackHandler handler = new NameClassPairMapperClosureAdapter(
                m_rowInserter);
        runSearch(handler, 0);
        m_rowInserter.afterInserting();
    }

    public List<UserPreview> getExample() {
        return search(m_previewSize);
    }

    public void dumpExample(Writer out) {
        try {
            CsvWriter writer = new CsvWriter(out);
            String[] allNames = Index.getAllNames();
            writer.write(allNames, false);

            Iterator<UserPreview> result = search(0).iterator();
            while (result.hasNext()) {
                UserPreview preview = result.next();
                String groupNamesString = StringUtils.join(preview.getGroupNames().iterator(),
                        ", ");
                String[] row = new String[allNames.length];
                Index.USERNAME.set(row, preview.getUser().getUserName());
                Index.FIRST_NAME.set(row, preview.getUser().getFirstName());
                Index.LAST_NAME.set(row, preview.getUser().getLastName());
                Index.ALIAS.set(row, preview.getUser().getAliasesString());
                Index.SIP_PASSWORD.set(row, preview.getUser().getSipPassword());
                Index.EMAIL.set(row, preview.getUser().getEmailAddress());
                Index.USER_GROUP.set(row, groupNamesString);
                Index.IM_ID.set(row, preview.getUser().getImId());

                Index.JOB_TITLE.set(row, preview.getUser().getCreatedAddressBookEntry().getJobTitle());
                Index.JOB_DEPT.set(row, preview.getUser().getCreatedAddressBookEntry().getJobDept());
                Index.COMPANY_NAME.set(row, preview.getUser().getCreatedAddressBookEntry().getCompanyName());
                Index.ASSISTANT_NAME.set(row, preview.getUser().getCreatedAddressBookEntry().getAssistantName());
                Index.CELL_PHONE_NUMBER
                        .set(row, preview.getUser().getCreatedAddressBookEntry().getCellPhoneNumber());
                Index.HOME_PHONE_NUMBER
                        .set(row, preview.getUser().getCreatedAddressBookEntry().getHomePhoneNumber());
                Index.ASSISTANT_PHONE_NUMBER.set(row, preview.getUser().getCreatedAddressBookEntry()
                        .getAssistantPhoneNumber());
                Index.FAX_NUMBER.set(row, preview.getUser().getCreatedAddressBookEntry().getFaxNumber());
                Index.ALTERNATE_EMAIL.set(row, preview.getUser().getCreatedAddressBookEntry()
                        .getAlternateEmailAddress());
                Index.ALTERNATE_IM_ID.set(row, preview.getUser().getCreatedAddressBookEntry().getAlternateImId());
                Index.LOCATION.set(row, preview.getUser().getCreatedAddressBookEntry().getLocation());
                Index.HOME_STREET.set(row, preview.getUser().getCreatedAddressBookEntry().getHomeAddress()
                        .getStreet());
                Index.HOME_CITY.set(row, preview.getUser().getCreatedAddressBookEntry().getHomeAddress().getCity());
                Index.HOME_STATE
                        .set(row, preview.getUser().getCreatedAddressBookEntry().getHomeAddress().getState());
                Index.HOME_COUNTRY.set(row, preview.getUser().getCreatedAddressBookEntry().getHomeAddress()
                        .getCountry());
                Index.HOME_ZIP.set(row, preview.getUser().getCreatedAddressBookEntry().getHomeAddress().getZip());
                Index.OFFICE_STREET.set(row, preview.getUser().getCreatedAddressBookEntry().getOfficeAddress()
                        .getStreet());
                Index.OFFICE_CITY.set(row, preview.getUser().getCreatedAddressBookEntry().getOfficeAddress()
                        .getCity());
                Index.OFFICE_STATE.set(row, preview.getUser().getCreatedAddressBookEntry().getOfficeAddress()
                        .getState());
                Index.OFFICE_COUNTRY.set(row, preview.getUser().getCreatedAddressBookEntry().getOfficeAddress()
                        .getCountry());
                Index.OFFICE_ZIP
                        .set(row, preview.getUser().getCreatedAddressBookEntry().getOfficeAddress().getZip());

                writer.write(row, true);
            }
        } catch (IOException e) {
            throw new UserException(e.getMessage());
        }
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

    private List<UserPreview> search(long limit) {
        LdapTemplate template = m_templateFactory.getLdapTemplate();
        m_userMapper.setAttrMap(m_ldapManager.getAttrMap());
        CollectingNameClassPairCallbackHandler handler = new NameClassPairMapperCollector(
                template, m_userMapper);
        runSearch(handler, limit);
        List<UserPreview> result = handler.getList();
        return result;
    }

    private void runSearch(NameClassPairCallbackHandler handler, long limit) {
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

        LdapTemplate template = m_templateFactory.getLdapTemplate();
        try {
            template.search(base, filter, sc, handler, LdapManager.NULL_PROCESSOR);
        } catch (SearchLimitExceededException normal) {
            // See http://forum.springframework.org/archive/index.php/t-27836.html
            LOG.debug("Normal overflow, requesting to preview more records then exist");
        } catch (DataAccessException e) {
            throw new UserException("LDAP search failed : " + e.getCause().getMessage());
        }
    }

    static class NameClassPairMapperClosureAdapter implements NameClassPairCallbackHandler {
        private Closure m_closure;

        NameClassPairMapperClosureAdapter(Closure closure) {
            m_closure = closure;
        }

        public void handleNameClassPair(NameClassPair nameClassPair) {
            m_closure.execute(nameClassPair);
        }
    }

    static class NameClassPairMapperCollector extends CollectingNameClassPairCallbackHandler {
        private NameClassPairMapper m_mapper;
        private LdapTemplate m_template;

        public NameClassPairMapperCollector(LdapTemplate template, NameClassPairMapper mapper) {
            m_template = template;
            m_mapper = mapper;
        }

        public Object getObjectFromNameClassPair(NameClassPair nameClassPair) {
            try {
                return m_mapper.mapFromNameClassPair(nameClassPair);
            } catch (NamingException e) {
                throw m_template.getExceptionTranslator().translate(e);
            }
        }
    }

    public void setTemplateFactory(LdapTemplateFactory templateFactory) {
        m_templateFactory = templateFactory;
    }
}
