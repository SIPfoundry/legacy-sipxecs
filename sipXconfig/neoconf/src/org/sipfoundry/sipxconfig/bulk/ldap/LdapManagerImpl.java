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

import static org.springframework.dao.support.DataAccessUtils.singleResult;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
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
import org.sipfoundry.sipxconfig.common.CronSchedule;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.dao.DataAccessException;
import org.springframework.dao.DataIntegrityViolationException;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowCallbackHandler;
import org.springframework.ldap.AttributesMapper;
import org.springframework.ldap.UncategorizedLdapException;

/**
 * Maintains LDAP connection params, attribute maps and schedule LdapManagerImpl
 */
public class LdapManagerImpl extends SipxHibernateDaoSupport implements LdapManager, ApplicationContextAware {
    private static final Log LOG = LogFactory.getLog(LdapManagerImpl.class);
    private static final String QUERY_OVERRIDE_PIN =
            "SELECT value_storage_id, value from setting_value where path='ldap/overwrite_pin'";
    private static final String NAMING_CONTEXTS = "namingContexts";
    private static final String SUBSCHEMA_SUBENTRY = "subschemaSubentry";
    private LdapTemplateFactory m_templateFactory;
    private JdbcTemplate m_jdbcTemplate;
    private ApplicationContext m_applicationContext;

    @Override
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
            attrMap.setSearchBaseDefault(searchBase);
            String subschemaSubentry = results.get(attrNames[1]);
            attrMap.setSubschemaSubentry(subschemaSubentry);
        } catch (NamingException e) {
            verifyException("&readData.failed", e);
        } catch (DataAccessException e) {
            verifyException("&connection.failed", e);
        }
    }

    @Override
    public void saveOverwritePin(boolean overwrite) {
        OverwritePinBean existingOPB = retriveOverwritePin();
        ValueStorage storage = null;
        if (existingOPB == null) {
            storage = new ValueStorage();
        } else {
            storage = getHibernateTemplate().load(ValueStorage.class, existingOPB.getId());
        }
        storage.setSettingValue("ldap/overwrite_pin", String.valueOf(overwrite));
        getHibernateTemplate().saveOrUpdate(storage);
    }

    @Override
    public OverwritePinBean retriveOverwritePin() {
        final ArrayList<OverwritePinBean> values = new ArrayList<OverwritePinBean>();
        RowCallbackHandler rows = new RowCallbackHandler() {
            @Override
            public void processRow(ResultSet rs) throws SQLException {
                OverwritePinBean bean = new OverwritePinBean(rs.getInt("value_storage_id"), rs.getBoolean("value"));
                values.add(bean);
            }
        };
        m_jdbcTemplate.query(QUERY_OVERRIDE_PIN, rows);
        return values.isEmpty() ? null : values.get(0);
    }

    @Override
    public boolean verifyLdapConnection(LdapConnectionParams params) {
        LOG.debug("verifying LDAP connection: " + params.getFullHost());
        try {
            String[] attrNames = new String[] {
                NAMING_CONTEXTS, SUBSCHEMA_SUBENTRY
            };
            retrieveDefaultSearchBase(params, attrNames);
            return true;
        } catch (NamingException e) {
            return false;
        } catch (DataAccessException e) {
            return false;
        }
    }

    @Override
    public boolean verifyAllLdapConnections() {
        LOG.debug("verifying all LDAP connections");
        List<LdapConnectionParams> connParams = getAllConnectionParams();
        for (LdapConnectionParams params : connParams) {
            if (!verifyLdapConnection(params)) {
                continue;
            } else {
                return true;
            }
        }
        return false;
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

    @Override
    public Schema getSchema(String subschemaSubentry, LdapConnectionParams params) {
        try {
            SearchControls cons = new SearchControls();
            // only interested in the first result
            cons.setCountLimit(1);
            // set time limit for this search to 30 sec, should be sufficient even for large LDAPs
            cons.setTimeLimit(30000);

            SchemaMapper mapper = new SchemaMapper();
            cons.setReturningAttributes(mapper.getReturningAttributes());
            cons.setSearchScope(SearchControls.OBJECT_SCOPE);

            Schema schema = (Schema) m_templateFactory.getLdapTemplate(params).search(subschemaSubentry,
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

        @Override
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
        cons.setTimeLimit(30000);

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

        @Override
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

    @Override
    public CronSchedule getSchedule(int connectionId) {
        return getConnectionParams(connectionId).getSchedule();
    }

    @Override
    public void setSchedule(CronSchedule schedule, int connectionId) {
        if (!schedule.isNew()) {
            // XCF-1168 incoming schedule is probably an update to this schedule
            // so add this schedule to cache preempt hibernate error about
            // multiple
            // objects with same ID in session. This is only true because
            // schedule
            // is managed by LdapConnectionParams object.
            getHibernateTemplate().update(schedule);
        }

        LdapConnectionParams connectionParams = getConnectionParams(connectionId);
        connectionParams.setSchedule(schedule);
        getHibernateTemplate().update(connectionParams);
        getDaoEventPublisher().publishSave(connectionParams);
        m_applicationContext.publishEvent(new LdapImportTrigger.ScheduleChangedEvent(schedule, this, connectionId));
    }

    @Override
    public AttrMap getAttrMap(int connectionId) {
        List<AttrMap> connectionsAttrMap = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "ldapConnectionAttrMap", "attrMapId", connectionId);
        if (!connectionsAttrMap.isEmpty()) {
            return connectionsAttrMap.get(0);
        }
        return createAttrMap();
    }

    @Override
    public LdapConnectionParams getConnectionParams(int connectionId) {
        List<LdapConnectionParams> connections = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "ldapConnection", "connectionId", connectionId);
        if (!connections.isEmpty()) {
            return connections.get(0);
        }
        return createConnectionParams();
    }

    @Override
    public LdapConnectionParams createConnectionParams() {
        LdapConnectionParams params = m_applicationContext.getBean(
                "ldapConnectionParams", LdapConnectionParams.class);
        return params;
    }

    @Override
    public AttrMap createAttrMap() {
        return m_applicationContext.getBean("attrMap", AttrMap.class);
    }

    @Override
    public List<LdapConnectionParams> getAllConnectionParams() {
        return getHibernateTemplate().loadAll(LdapConnectionParams.class);
    }

    @Override
    public void setAttrMap(AttrMap attrMap) {
        getHibernateTemplate().merge(attrMap);
        getDaoEventPublisher().publishSave(attrMap);
    }

    @Override
    public void saveSystemSettings(LdapSystemSettings settings) {
        getHibernateTemplate().saveOrUpdate(settings);
    }

    @Override
    public LdapSystemSettings getSystemSettings() {
        List settingses = getHibernateTemplate().loadAll(LdapSystemSettings.class);
        LdapSystemSettings settings = (LdapSystemSettings) singleResult(settingses);
        if (settings == null) {
            settings = m_applicationContext.getBean("ldapSystemSettings",
                                            LdapSystemSettings.class);
        }
        return settings;
    }

    @Override
    public void setConnectionParams(LdapConnectionParams params) {
        getHibernateTemplate().saveOrUpdate(params);
        getDaoEventPublisher().publishSave(params);
    }

    @Override
    public void removeConnectionParams(int connectionId) {
        LdapConnectionParams params = getConnectionParams(connectionId);
        getHibernateTemplate().delete(params);
        getHibernateTemplate().delete(getAttrMap(connectionId));
        getDaoEventPublisher().publishDelete(params);
        m_applicationContext.publishEvent(new LdapImportTrigger.ScheduleDeletedEvent(this, connectionId));
    }

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }

    public void setTemplateFactory(LdapTemplateFactory templateFactory) {
        m_templateFactory = templateFactory;
    }

    @Required
    public void setConfigJdbcTemplate(JdbcTemplate template) {
        m_jdbcTemplate = template;
    }
}
