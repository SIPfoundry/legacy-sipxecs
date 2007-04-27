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
import javax.naming.directory.DirContext;
import javax.naming.directory.SearchControls;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.CronSchedule;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.ldap.AttributesMapper;
import org.springframework.ldap.DefaultNameClassPairMapper;
import org.springframework.ldap.DirContextProcessor;
import org.springframework.ldap.LdapTemplate;

/**
 * Maintains LDAP connection params, attribute maps and schedule LdapManagerImpl
 */
public class LdapManagerImpl extends SipxHibernateDaoSupport implements LdapManager, ApplicationContextAware {
    public static final String FILTER_ALL_CLASSES = "objectclass=*";

    private static final Log LOG = LogFactory.getLog(LdapManagerImpl.class);
    private static final String LDAP_TEMPLATE_BEAN_ID = "ldapTemplate";
    private static final String LDAP_CONTEXT_SOURCE_BEAN_ID = "ldapContextSource";

    private ApplicationContext m_applicationContext;

    public void verify(LdapConnectionParams params, AttrMap attrMap) {
        try {
            String searchBase = retrieveDefaultSearchBase(params);
            // it will only overwrite the search base if not set yet
            if (StringUtils.isBlank(attrMap.getSearchBase())) {
                attrMap.setSearchBase(searchBase);
            }
        } catch (NamingException e) {
            LOG.debug("Verifying LDAP connection failed.", e);
            throw new UserException("Cannot connect to LDAP server: " + e.getMessage());
        }
    }

    public Schema getSchema() {
        try {
            return retrieveSchema();
        } catch (NamingException e) {
            LOG.debug("Retrieving schema failed.", e);
            throw new UserException("Cannot retrieve schema from LDAP server: " + e.getMessage());
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
        List<Attributes> results = getLdapTemplate(params).search("", FILTER_ALL_CLASSES, cons, 
                new AttributesPassThru(), new NullDirContextProcessor());
        // only interested in the first result
        if (results.size() > 0) {
            return (String) results.get(0).get(attrs[0]).get();
        }
        return StringUtils.EMPTY;
    }
    
    private final class NullDirContextProcessor implements DirContextProcessor {
        public void postProcess(DirContext ctx) throws NamingException {
        }

        public void preProcess(DirContext ctx) throws NamingException {
        }
    }
    
    static class AttributesPassThru implements AttributesMapper {
        public Object mapFromAttributes(Attributes attributes) throws NamingException {
            return attributes;
        }    
    }

    private Schema retrieveSchema() throws NamingException {
        SearchControls cons = new SearchControls();
        String[] attrs = new String[] {
            "objectClasses"
        };

        cons.setReturningAttributes(attrs);
        cons.setSearchScope(SearchControls.OBJECT_SCOPE);
        
        new DefaultNameClassPairMapper();
        
        List<Attributes> results = getLdapTemplate().search("cn=subSchema", FILTER_ALL_CLASSES,
                cons, new AttributesPassThru(), new NullDirContextProcessor());
        // only interested in the first result

        Schema schema = new Schema();
        if (results.size() == 0) {
            return schema;
        }
        Attributes result = results.get(0);
        NamingEnumeration definitions = result.get(attrs[0]).getAll();
        while (definitions.hasMoreElements()) {
            String classDefinition = (String) definitions.nextElement();
            schema.addClassDefinition(classDefinition);
        }
        return schema;
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

    public LdapTemplate getLdapTemplate() {
        return (LdapTemplate) m_applicationContext.getBean(LDAP_TEMPLATE_BEAN_ID);
    }

    public LdapTemplate getLdapTemplate(LdapConnectionParams params) {
        LdapTemplate template = getLdapTemplate();        
        ContextSourceFromConnectionParams source = (ContextSourceFromConnectionParams) 
            m_applicationContext.getBean(LDAP_CONTEXT_SOURCE_BEAN_ID);
        params.applyToContext(source);
        source.applyParameters(params);
        template.setContextSource(source);
        return template;
    }
    
    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }
}
