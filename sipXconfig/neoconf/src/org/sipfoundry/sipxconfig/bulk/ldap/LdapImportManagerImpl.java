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
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.bulk.UserPreview;
import org.sipfoundry.sipxconfig.bulk.csv.Index;
import org.sipfoundry.sipxconfig.bulk.csv.SimpleCsvWriter;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.ldap.SizeLimitExceededException;
import org.springframework.ldap.control.PagedResultsDirContextProcessor;
import org.springframework.ldap.core.CollectingNameClassPairCallbackHandler;
import org.springframework.ldap.core.LdapTemplate;
import org.springframework.ldap.core.NameClassPairMapper;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class LdapImportManagerImpl extends HibernateDaoSupport implements LdapImportManager {
    public static final Log LOG = LogFactory.getLog(LdapImportManager.class);
    private LdapTemplateFactory m_templateFactory;
    private LdapManager m_ldapManager;
    private LdapRowInserter m_rowInserter;
    private UserMapper m_userMapper;
    private AdminContext m_adminContext;
    private int m_previewSize;

    public void setUserMapper(UserMapper userMapper) {
        m_userMapper = userMapper;
    }

    @Override
    public void insert(int connectionId) {
        LOG.info("***** START INSERTING DATA *****");
        try {
            m_rowInserter.setAttrMap(m_ldapManager.getAttrMap(connectionId));
            m_rowInserter.setDomain(m_ldapManager.getConnectionParams(connectionId).getDomain());
            m_rowInserter.beforeInserting(null);
            CollectingNameClassPairCallbackHandler handler = new NameClassPairMapperClosureAdapter(m_rowInserter);
            runSearch(0, handler, connectionId);
            m_rowInserter.afterInserting();
        } catch (Exception ex) {
            LOG.error("***** FAILURE DURING INSERTING DATA *****", ex);
        }
    }

    @Override
    public List<UserPreview> getExample(int connectionId) {
        return searchPreview(m_previewSize, connectionId);
    }

    @Override
    public void dumpExample(List<UserPreview> list, Writer out, int connectionId) {
        try {
            SimpleCsvWriter writer = new SimpleCsvWriter(out);
            String[] allNames = Index.getAllNames();
            writer.write(allNames, false);

            Iterator<UserPreview> result = list.iterator();
            while (result.hasNext()) {
                UserPreview preview = result.next();
                String groupNamesString = StringUtils.join(preview.getGroupNames().iterator(),
                        ", ");
                String[] row = new String[allNames.length];
                Index.USERNAME.set(row, preview.getUser().getUserName());
                Index.AUTH_ACCOUNT_NAME.set(row, preview.getUser().getAuthAccountName());
                Index.FIRST_NAME.set(row, preview.getUser().getFirstName());
                Index.LAST_NAME.set(row, preview.getUser().getLastName());
                Index.ALIAS.set(row, preview.getUser().getAliasesString());
                Index.SIP_PASSWORD.set(row, preview.getUser().getSipPassword());
                Index.EMAIL.set(row, preview.getUser().getEmailAddress());
                Index.USER_GROUP.set(row, groupNamesString);
                Index.IM_ID.set(row, preview.getUser().getImId());

                Index.JOB_TITLE.set(row, preview.getUser().getUserProfile().getJobTitle());
                Index.JOB_DEPT.set(row, preview.getUser().getUserProfile().getJobDept());
                Index.COMPANY_NAME.set(row, preview.getUser().getUserProfile().getCompanyName());
                Index.ASSISTANT_NAME.set(row, preview.getUser().getUserProfile().getAssistantName());
                Index.CELL_PHONE_NUMBER.set(row, preview.getUser().getUserProfile().getCellPhoneNumber());
                Index.HOME_PHONE_NUMBER.set(row, preview.getUser().getUserProfile().getHomePhoneNumber());
                Index.ASSISTANT_PHONE_NUMBER.set(row, preview.getUser().getUserProfile().getAssistantPhoneNumber());
                Index.FAX_NUMBER.set(row, preview.getUser().getUserProfile().getFaxNumber());
                Index.ALTERNATE_EMAIL.set(row, preview.getUser().getUserProfile().getAlternateEmailAddress());
                Index.ALTERNATE_IM_ID.set(row, preview.getUser().getUserProfile().getAlternateImId());
                Index.LOCATION.set(row, preview.getUser().getUserProfile().getLocation());
                Index.HOME_STREET.set(row, preview.getUser().getUserProfile().getHomeAddress().getStreet());
                Index.HOME_CITY.set(row, preview.getUser().getUserProfile().getHomeAddress().getCity());
                Index.HOME_STATE.set(row, preview.getUser().getUserProfile().getHomeAddress().getState());
                Index.HOME_COUNTRY.set(row, preview.getUser().getUserProfile().getHomeAddress().getCountry());
                Index.HOME_ZIP.set(row, preview.getUser().getUserProfile().getHomeAddress().getZip());
                Index.OFFICE_STREET.set(row, preview.getUser().getUserProfile().getOfficeAddress().getStreet());
                Index.OFFICE_CITY.set(row, preview.getUser().getUserProfile().getOfficeAddress().getCity());
                Index.OFFICE_STATE.set(row, preview.getUser().getUserProfile().getOfficeAddress().getState());
                Index.OFFICE_COUNTRY.set(row, preview.getUser().getUserProfile().getOfficeAddress().getCountry());
                Index.OFFICE_ZIP.set(row, preview.getUser().getUserProfile().getOfficeAddress().getZip());
                writer.write(row);
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

    @Required
    public void setPreviewSize(int previewSize) {
        m_previewSize = previewSize;
    }

    private Integer getPageImportSize() {
        return m_adminContext.getPageImportSize();
    }

    private List<UserPreview> searchPreview(int limit, int connectionId) {
        try {
            LdapTemplate template = m_templateFactory.getLdapTemplate(m_ldapManager.getConnectionParams(connectionId));
            m_userMapper.setAttrMap(m_ldapManager.getAttrMap(connectionId));
            CollectingNameClassPairCallbackHandler handler = new NameClassPairMapperCollector(m_userMapper);
            runSearch(limit, handler, connectionId);
            List<UserPreview> result = handler.getList();
            return result;
        } catch (Exception ex) {
            LOG.error("LDAP preview search failed", ex);
            throw new UserException("LDAP preview search failed : " + ex.getMessage());
        }
    }

    private void runSearch(int limit, CollectingNameClassPairCallbackHandler handler, int connectionId) {
        SearchControls sc = new SearchControls();
        sc.setSearchScope(SearchControls.SUBTREE_SCOPE);
        int pageImportSize = getPageImportSize();
        AttrMap attrMap = m_ldapManager.getAttrMap(connectionId);
        if (!attrMap.verified()) {
            m_ldapManager.verify(m_ldapManager.getConnectionParams(connectionId), attrMap);
        }

        sc.setReturningAttributes(attrMap.getLdapAttributesArray());

        String base = attrMap.getSearchBase();
        String filter = attrMap.getSearchFilter();
        int searched = 0;
        LdapTemplate template = m_templateFactory.getLdapTemplate(m_ldapManager.getConnectionParams(connectionId));
        try {
            PagedResultsDirContextProcessor processor = null;
            do {
                int pageNo = (limit > 0 && limit <= pageImportSize) ? limit : pageImportSize;
                if (limit > 0 && limit - searched > 0 && limit - searched < pageNo) {
                    pageNo = limit - searched;
                }
                processor = new PagedResultsDirContextProcessor(pageNo,
                        processor != null ? processor.getCookie() : null);
                template.search(base, filter, sc, handler, processor);
                searched = handler.getList().size();
                LOG.info("Number of processed LDAP users is: "
                        + searched + " current page size is: "
                        + processor.getPageSize() + " search limit is (0 means all): " + limit);
                if ((limit > 0 && searched >= limit)) {
                    break;
                }
            } while(processor.getCookie().getCookie() != null);
        } catch (Exception e) {
            if (e instanceof SizeLimitExceededException) {
                // See http://forum.springframework.org/archive/index.php/t-27836.html
                LOG.info("Normal overflow, requesting to preview more records then exist", e);
            } else {
                LOG.error("LDAP search failed", e);
                throw new UserException("LDAP search failed : " + e.getCause().getMessage());
            }
        }
    }

    static class NameClassPairMapperClosureAdapter extends CollectingNameClassPairCallbackHandler {
        private Closure m_closure;

        NameClassPairMapperClosureAdapter(Closure closure) {
            m_closure = closure;
        }

        @Override
        public Object getObjectFromNameClassPair(NameClassPair nameClassPair) {
            try {
                m_closure.execute(nameClassPair);
            } catch (Exception e) {
                LOG.error("Error processing " + nameClassPair);
            }
            return nameClassPair;
        }
    }

    static class NameClassPairMapperCollector  extends CollectingNameClassPairCallbackHandler {
        private NameClassPairMapper m_mapper;

        public NameClassPairMapperCollector(NameClassPairMapper mapper) {
            m_mapper = mapper;
        }

        @Override
        public Object getObjectFromNameClassPair(NameClassPair nameClassPair) {
            try {
                return m_mapper.mapFromNameClassPair(nameClassPair);
            } catch (NamingException e) {
                throw new UserException(e);
            }
        }
    }

    public void setTemplateFactory(LdapTemplateFactory templateFactory) {
        m_templateFactory = templateFactory;
    }

    @Required
    public void setAdminContext(AdminContext adminContext) {
        m_adminContext = adminContext;
    }
}
