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

import java.util.List;

import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.directory.Attributes;
import javax.naming.directory.SearchControls;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.CronSchedule;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.dao.DataAccessException;
import org.springframework.dao.DataIntegrityViolationException;
import org.springframework.ldap.AttributesMapper;

/**
 * Maintains LDAP connection params, attribute maps and schedule LdapManagerImpl
 */
public class LdapManagerImpl extends SipxHibernateDaoSupport implements LdapManager, ApplicationContextAware {
    private static final Log LOG = LogFactory.getLog(LdapManagerImpl.class);
    private LdapTemplateFactory m_templateFactory;
    private ApplicationContext m_applicationContext;

    public void verify(LdapConnectionParams params, AttrMap attrMap) {
        try {
            String searchBase = retrieveDefaultSearchBase(params);
            // it will only overwrite the search base if not set yet
            if (StringUtils.isBlank(attrMap.getSearchBase())) {
                attrMap.setSearchBase(searchBase);
            }
        } catch (NamingException e) {
            verifyException("Failed to read data LDAP information : ", e);
        } catch (DataAccessException e) {
            verifyException("Failed connection to LDAP server : ", e);
        }
    }
    
    private void verifyException(String message, Exception e) {
        LOG.debug("Verifying LDAP connection failed.", e);
        throw new UserException(message + e.getMessage());        
    }

    public Schema getSchema() {
        try {
            SearchControls cons = new SearchControls();
            // only interested in the first result
            cons.setCountLimit(1);
    
            SchemaMapper mapper = new SchemaMapper();
            cons.setReturningAttributes(mapper.getReturningAttributes());
            cons.setSearchScope(SearchControls.OBJECT_SCOPE);        
    
            Schema schema = (Schema) m_templateFactory.getLdapTemplate().search("cn=subSchema",
                    LdapManager.FILTER_ALL_CLASSES, cons, new SchemaMapper(), LdapManager.NULL_PROCESSOR).get(0);
            
            
            return schema;
        } catch (DataIntegrityViolationException e) {
            LOG.debug("Retrieving schema failed.", e);
            throw new UserException("Cannot retrieve schema from LDAP server: " + e.getMessage());            
        }
    }

    private static class SchemaMapper implements AttributesMapper {
        private Schema m_schema;
        private boolean m_initialized;        
        
        SchemaMapper() {
            m_schema = new Schema();
        }
        
        public String[] getReturningAttributes() {
            return new String[] { 
                "objectClasses"
            };            
        }
    
        public Object mapFromAttributes(Attributes attributes) throws NamingException {
            // only interested in the first result
            if (!m_initialized) {
                NamingEnumeration definitions = attributes.get(getReturningAttributes()[0]).getAll();
                while (definitions.hasMoreElements()) {
                    String classDefinition = (String) definitions.nextElement();
                    m_schema.addClassDefinition(classDefinition);
                }
                m_initialized = true;
            }
            return m_schema;
        }
    }

    /**
     * Connects to LDAP to retrieve the namingContexts attribute from root. Good way to verify if
     * LDAP is accessible. Command line anologue is:
     * 
     * ldapsearch -x -b '' -s base '(objectclass=*)' namingContexts
     * 
     * @return namingContext value - can be used as the search base for user if nothing more
     *         specific is provided
     * @throws NamingException
     */
    private String retrieveDefaultSearchBase(LdapConnectionParams params) throws NamingException {
        SearchControls cons = new SearchControls();
        String[] attrs = new String[] {
            "namingContexts"
        };

        cons.setReturningAttributes(attrs);
        cons.setSearchScope(SearchControls.OBJECT_SCOPE);
        List<Attributes> results = m_templateFactory.getLdapTemplate(params).search("", FILTER_ALL_CLASSES, cons, 
                new AttributesPassThru(), NULL_PROCESSOR);
        // only interested in the first result
        if (results.size() > 0) {
            return (String) results.get(0).get(attrs[0]).get();
        }
        return StringUtils.EMPTY;
    }
    
    
    static class AttributesPassThru implements AttributesMapper {
        public Object mapFromAttributes(Attributes attributes) throws NamingException {
            return attributes;
        }    
    }

    public CronSchedule getSchedule() {
        return getConnectionParams().getSchedule();
    }

    public void setSchedule(CronSchedule schedule) {
        if (!schedule.isNew()) {
            // XCF-1168 incoming schedule is probably an update to this schedule
            // so add this schedule to cache preempt hibernate error about multiple
            // objects with same ID in session. This is only true because schedule
            // is managed by LdapConnectionParams object.
            getHibernateTemplate().update(schedule);
        }

        LdapConnectionParams connectionParams = getConnectionParams();
        connectionParams.setSchedule(schedule);
        getHibernateTemplate().update(connectionParams);

        m_applicationContext.publishEvent(new LdapImportTrigger.ScheduleChangedEvent(schedule, this));
    }

    public AttrMap getAttrMap() {
        List<AttrMap> connections = getHibernateTemplate().loadAll(AttrMap.class);
        if (!connections.isEmpty()) {
            return connections.get(0);
        }
        AttrMap attrMap = (AttrMap) m_applicationContext.getBean("attrMap", AttrMap.class);
        getHibernateTemplate().save(attrMap);
        return attrMap;
    }

    public LdapConnectionParams getConnectionParams() {
        List<LdapConnectionParams> connections = getHibernateTemplate().loadAll(LdapConnectionParams.class);
        if (!connections.isEmpty()) {
            return connections.get(0);
        }
        LdapConnectionParams params = (LdapConnectionParams) m_applicationContext.getBean(
                "ldapConnectionParams", LdapConnectionParams.class);
        getHibernateTemplate().save(params);
        return params;
    }

    public void setAttrMap(AttrMap attrMap) {
        getHibernateTemplate().saveOrUpdate(attrMap);
    }

    public void setConnectionParams(LdapConnectionParams params) {
        getHibernateTemplate().saveOrUpdate(params);
    }

    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }

    public void setTemplateFactory(LdapTemplateFactory templateFactory) {
        m_templateFactory = templateFactory;
    }
}
