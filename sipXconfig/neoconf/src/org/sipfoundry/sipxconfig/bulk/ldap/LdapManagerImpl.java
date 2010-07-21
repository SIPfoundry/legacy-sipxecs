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

import java.util.HashMap;
import java.util.List;
import java.util.Map;

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
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.dao.DataAccessException;
import org.springframework.dao.DataIntegrityViolationException;
import org.springframework.ldap.AttributesMapper;
import org.springframework.ldap.UncategorizedLdapException;

import static org.springframework.dao.support.DataAccessUtils.singleResult;

/**
 * Maintains LDAP connection params, attribute maps and schedule LdapManagerImpl
 */
public class LdapManagerImpl extends SipxHibernateDaoSupport implements LdapManager, ApplicationContextAware {
    private static final Log LOG = LogFactory.getLog(LdapManagerImpl.class);
    private static final String NAMING_CONTEXTS = "namingContexts";
    private static final String SUBSCHEMA_SUBENTRY = "subschemaSubentry";
    private LdapTemplateFactory m_templateFactory;
    private ApplicationContext m_applicationContext;
    private SipxServiceManager m_sipxServiceManager;
    private ServiceConfigurator m_serviceConfigurator;

    public void verify(LdapConnectionParams params, AttrMap attrMap) {
        try {
            String[] attrNames = new String[] {
                NAMING_CONTEXTS, SUBSCHEMA_SUBENTRY
            };
            Map<String, String> results = retrieveDefaultSearchBase(params, attrNames);

            String searchBase = results.get(attrNames[0]).trim();
            // it will only overwrite the search base if not set yet
            if (StringUtils.isBlank(attrMap.getSearchBase()) && StringUtils.isNotEmpty(searchBase)) {
                attrMap.setSearchBase(searchBase);
            }
            String subschemaSubentry = results.get(attrNames[1]);
            attrMap.setSubschemaSubentry(subschemaSubentry);
        } catch (NamingException e) {
            verifyException("&readData.failed", e);
        } catch (DataAccessException e) {
            verifyException("&connection.failed", e);
        }
    }

    public boolean verifyLdapConnection() {
        try {
            String[] attrNames = new String[] {
                NAMING_CONTEXTS, SUBSCHEMA_SUBENTRY
            };
            retrieveDefaultSearchBase(getConnectionParams(), attrNames);
            return true;
        } catch (NamingException e) {
            return false;
        } catch (DataAccessException e) {
            return false;
        }
    }

    private void verifyException(String message, Exception e) {
        LOG.debug("Verifying LDAP connection failed.", e);
        // for the purpose of readability, remove any nested exception
        // info from the message, if there is any. This is done by only
        // taking that part of the message up to the first ';'
        String fullMessage = e.getMessage();
        String parsedMessage = fullMessage;
        if (fullMessage.indexOf(';') > 0) {
            parsedMessage = fullMessage.substring(0, fullMessage.indexOf(';'));
        }
        throw new UserException(message, parsedMessage);
    }

    public Schema getSchema(String subschemaSubentry) {
        try {
            SearchControls cons = new SearchControls();
            // only interested in the first result
            cons.setCountLimit(1);

            SchemaMapper mapper = new SchemaMapper();
            cons.setReturningAttributes(mapper.getReturningAttributes());
            cons.setSearchScope(SearchControls.OBJECT_SCOPE);

            Schema schema = (Schema) m_templateFactory.getLdapTemplate().search(subschemaSubentry,
                                            LdapManager.FILTER_ALL_CLASSES, cons, new SchemaMapper(),
                                            LdapManager.NULL_PROCESSOR).get(0);

            return schema;
        } catch (DataIntegrityViolationException e) {
            LOG.debug("Retrieving schema failed.", e);
            throw new UserException("searchSchema.violation.error");
        } catch (UncategorizedLdapException e) {
            LOG.debug("Retrieving schema failed. Anonymous-binding may be disabled", e);
            throw new UserException("searchSchema.anonymousBinding.error");
        }
    }

    private static class SchemaMapper implements AttributesMapper {
        private final Schema m_schema;
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
     * Connects to LDAP to retrieve the namingContexts attribute from root. Good
     * way to verify if LDAP is accessible. Command line anologue is:
     *
     * ldapsearch -x -b '' -s base '(objectclass=*)' namingContexts
     *
     * @param attrNames
     *            TODO
     *
     * @return namingContext value - can be used as the search base for user if
     *         nothing more specific is provided
     * @throws NamingException
     */
    private Map<String, String> retrieveDefaultSearchBase(LdapConnectionParams params, String[] attrNames)
        throws NamingException {

        SearchControls cons = new SearchControls();

        cons.setReturningAttributes(attrNames);
        cons.setSearchScope(SearchControls.OBJECT_SCOPE);

        List<Map<String, String>> results = m_templateFactory.getLdapTemplate(params).search("",
                FILTER_ALL_CLASSES, cons, new AttributesToValues(attrNames), NULL_PROCESSOR);
        // only interested in the first result
        if (results.size() > 0) {
            return results.get(0);
        }
        return null;
    }

    static class AttributesToValues implements AttributesMapper {
        private final String[] m_attrNames;

        public AttributesToValues(String... attrNames) {
            m_attrNames = attrNames;
        }

        public Map<String, String> mapFromAttributes(Attributes attributes) throws NamingException {
            Map<String, String> values = new HashMap<String, String>();
            for (String attrName : m_attrNames) {
                // only retrieves a single value for each attribute
                Object value = attributes.get(attrName).get();
                values.put(attrName, value.toString());
            }
            return values;
        }
    }

    public CronSchedule getSchedule() {
        return getConnectionParams().getSchedule();
    }

    public void setSchedule(CronSchedule schedule) {
        if (!schedule.isNew()) {
            // XCF-1168 incoming schedule is probably an update to this schedule
            // so add this schedule to cache preempt hibernate error about
            // multiple
            // objects with same ID in session. This is only true because
            // schedule
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

    public void saveSystemSettings(LdapSystemSettings settings) {
        getHibernateTemplate().saveOrUpdate(settings);
    }

    public LdapSystemSettings getSystemSettings() {
        List settingses = getHibernateTemplate().loadAll(LdapSystemSettings.class);
        LdapSystemSettings settings = (LdapSystemSettings) singleResult(settingses);
        if (settings == null) {
            settings = (LdapSystemSettings) m_applicationContext.getBean("ldapSystemSettings",
                                            LdapSystemSettings.class);
        }
        return settings;
    }

    public void setConnectionParams(LdapConnectionParams params) {
        getHibernateTemplate().saveOrUpdate(params);
    }

    public void replicateOpenfireConfig() {
        SipxService openfireService = m_sipxServiceManager.getServiceByBeanId("sipxOpenfireService");
        m_serviceConfigurator.replicateServiceConfig(openfireService);
    }

    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }

    public void setTemplateFactory(LdapTemplateFactory templateFactory) {
        m_templateFactory = templateFactory;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }
}
