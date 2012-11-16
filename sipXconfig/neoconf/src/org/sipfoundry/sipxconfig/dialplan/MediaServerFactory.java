/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan;

import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.support.rowset.SqlRowSet;

public class MediaServerFactory implements BeanFactoryAware {
    /** Spring bean name that defines default media server */
    private static final String DEFAULT_MEDIA_SERVER = "freeswitchMediaServer";
    private static final String EXCHANGE_MEDIA_SERVER = "exchangeUmMediaServer";

    private BeanFactory m_beanFactory;
    private Collection<String> m_beanIds;
    private JdbcTemplate m_jdbcTemplate;

    public MediaServer create(String id) {
        String mediaServerBeanId = StringUtils.defaultIfEmpty(id, DEFAULT_MEDIA_SERVER);
        MediaServer server = (MediaServer) m_beanFactory.getBean(mediaServerBeanId, MediaServer.class);
        if (mediaServerBeanId.equals(EXCHANGE_MEDIA_SERVER)) {
            populateExchangeServer(server);
        } else {
            server.setHostname(StringUtils.EMPTY);
            server.setServerExtension(StringUtils.EMPTY);
        }
        return server;
    }

    protected void populateExchangeServer(MediaServer server) {
        SqlRowSet queryForRowSet = m_jdbcTemplate.queryForRowSet(
                "select voice_mail, media_server_hostname from internal_dialing_rule where media_server_type = ?",
                EXCHANGE_MEDIA_SERVER);
        while (queryForRowSet.next()) {
            server.setHostname(queryForRowSet.getString("media_server_hostname"));
            server.setServerExtension(queryForRowSet.getString("voice_mail"));
        }
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void setBeanIds(Collection<String> beanIds) {
        m_beanIds = beanIds;
    }

    public Collection<String> getBeanIds() {
        return m_beanIds;
    }

    public Map<String, String> getBeanIdsToLabels() {
        LinkedHashMap<String, String> result = new LinkedHashMap<String, String>();
        for (String beanId : m_beanIds) {
            MediaServer ms = create(beanId);
            result.put(beanId, ms.getLabel());
        }
        return result;
    }

    public MediaServer createDefault() {
        return create(DEFAULT_MEDIA_SERVER);
    }

    public MediaServer createExchangeServer() {
        return create(EXCHANGE_MEDIA_SERVER);
    }

    public void setJdbcTemplate(JdbcTemplate jdbcTemplate) {
        m_jdbcTemplate = jdbcTemplate;
    }
}
