/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Order;
import org.hibernate.criterion.Projections;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.ExtensionInUseException;
import org.sipfoundry.sipxconfig.common.NameInUseException;
import org.sipfoundry.sipxconfig.common.SameExtensionException;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateCallback;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class ConferenceBridgeContextImpl extends SipxHibernateDaoSupport implements BeanFactoryAware,
        ConferenceBridgeContext, DaoEventListener {
    private static final String BUNDLE_CONFERENCE = "conference";
    private static final String CONFERENCE = "&label.conference";
    private static final String VALUE = "value";
    private static final String CONFERENCE_IDS_WITH_ALIAS = "conferenceIdsWithAlias";
    private static final String CONFERENCE_BY_NAME = "conferenceByName";
    private static final String OWNER = "owner";
    private static final String PERCENT = "%";

    private AliasManager m_aliasManager;
    private BeanFactory m_beanFactory;
    private DomainManager m_domainManager;

    public List<Bridge> getBridges() {
        return getHibernateTemplate().loadAll(Bridge.class);
    }

    public void saveBridge(Bridge bridge) {
        getHibernateTemplate().saveOrUpdate(bridge);
        if (bridge.isNew()) {
            // need to make sure that ID is set
            getHibernateTemplate().flush();
        }
    }

    public void saveConference(Conference conference) {
        validate(conference);
        if (conference.isNew()) {
            getHibernateTemplate().saveOrUpdate(conference);
        } else {
            getHibernateTemplate().merge(conference);
        }
    }

    public void validate(Conference conference) {
        String name = conference.getName();
        String extension = conference.getExtension();
        String did = conference.getDid();
        if (name == null) {
            throw new UserException("A conference must have a name");
        }
        if (extension == null) {
            throw new UserException("A conference must have an extension");
        }

        if (conference.getModeratorAccessCode() != null && conference.getParticipantAccessCode() == null) {
            throw new UserException("&error.moderator.and.participant.pin");
        }

        if (conference.getModeratorAccessCode() == null && !conference.isQuickstart()) {
            throw new UserException("&error.non.qs.no.mod");
        }

        if (!conference.isQuickstart() && StringUtils.equals(conference.getModeratorAccessCode(),
                conference.getParticipantAccessCode())) {
            throw new UserException("&error.moderator.eq.participant");
        }

        if (!m_aliasManager.canObjectUseAlias(conference, name)) {
            throw new NameInUseException(CONFERENCE, name);
        }
        if (!m_aliasManager.canObjectUseAlias(conference, extension)) {
            throw new ExtensionInUseException(CONFERENCE, extension);
        }
        if (!m_aliasManager.canObjectUseAlias(conference, did)) {
            throw new ExtensionInUseException(CONFERENCE, did);
        }
        if (StringUtils.isNotBlank(did) && did.equals(extension)) {
            throw new SameExtensionException("did", "extension");
        }
    }

    public Bridge newBridge() {
        return (Bridge) m_beanFactory.getBean(Bridge.BEAN_NAME, Bridge.class);
    }

    public Conference newConference() {
        Conference conference = (Conference) m_beanFactory.getBean(Conference.BEAN_NAME, Conference.class);
        conference.generateAccessCodes();
        return conference;
    }

    private void removeConferences(List<Conference> conferences) {
        Collection<Integer> ids = new ArrayList<Integer>();
        for (Conference conf : conferences) {
            ids.add(conf.getId());
        }
        removeConferences(ids);
    }

    public void removeConferences(Collection<Integer> conferencesIds) {
        Set<Bridge> bridges = new HashSet<Bridge>();
        for (Iterator<Integer> i = conferencesIds.iterator(); i.hasNext();) {
            Serializable id = i.next();
            Conference conference = loadConference(id);
            getDaoEventPublisher().publishDelete(conference);
            Bridge bridge = conference.getBridge();
            bridge.removeConference(conference);
            bridges.add(bridge);
        }
        getHibernateTemplate().saveOrUpdateAll(bridges);
        getHibernateTemplate().flush();
    }

    public Bridge loadBridge(Serializable id) {
        return (Bridge) getHibernateTemplate().load(Bridge.class, id);
    }

    public Bridge getBridgeByServer(String hostname) {
        // TODO JPA This is temporarily commented out until I can figure out why loading the
        // object in this way
        // does not load dependent objects like the service and location...
        // List<Bridge> bridges = getHibernateTemplate().findByNamedQueryAndNamedParam(
        // "bridgeByHost", VALUE, hostname);
        // return (Bridge) DataAccessUtils.singleResult(bridges2);

        Bridge bridgeForServer = null;
        List<Bridge> bridges = getHibernateTemplate().loadAll(Bridge.class);
        for (Bridge b : bridges) {
            if (b != null) {
                if (b.getLocation() != null) {
                    if (b.getLocation().getFqdn().equalsIgnoreCase(hostname)) {
                        bridgeForServer = b;
                        break;
                    }
                }
            }
        }
        return bridgeForServer;
    }

    public Conference loadConference(Serializable id) {
        return (Conference) getHibernateTemplate().load(Conference.class, id);
    }

    public Conference findConferenceByName(String name) {
        List<Conference> conferences = getHibernateTemplate().findByNamedQueryAndNamedParam(CONFERENCE_BY_NAME,
                VALUE, name);
        return (Conference) DataAccessUtils.singleResult(conferences);
    }

    public void clear() {
        List bridges = getBridges();
        getHibernateTemplate().deleteAll(bridges);
    }

    // trivial get/set
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public boolean isAliasInUse(String alias) {
        List confIds = getHibernateTemplate().findByNamedQueryAndNamedParam(CONFERENCE_IDS_WITH_ALIAS, VALUE, alias);
        return !confIds.isEmpty();
    }

    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection ids = getHibernateTemplate().findByNamedQueryAndNamedParam(CONFERENCE_IDS_WITH_ALIAS, VALUE,
                alias);
        Collection bids = BeanId.createBeanIdCollection(ids, Conference.class);
        return bids;
    }

    public List<Conference> findConferencesByOwner(User owner) {
        List<Conference> conferences = getHibernateTemplate().findByNamedQueryAndNamedParam("conferencesByOwner",
                OWNER, owner);
        return conferences;
    }

    private Criteria filterConferencesCriteria(final Integer bridgeId, final Integer ownerGroupId, Session session) {
        Criteria criteria = session.createCriteria(Conference.class);
        criteria.createCriteria("bridge", "b").add(Restrictions.eq("b.id", bridgeId));
        if (ownerGroupId != null) {
            criteria.createCriteria(OWNER, "o").createCriteria("groups", "g")
                    .add(Restrictions.eq("g.id", ownerGroupId));
        }
        return criteria;
    }

    public List<Conference> getAllConferences() {
        return getHibernateTemplate().loadAll(Conference.class);
    }

    public List<Conference> filterConferences(final Integer bridgeId, final Integer ownerGroupId) {
        HibernateCallback callback = new HibernateCallback() {
            public Object doInHibernate(Session session) {
                Criteria criteria = filterConferencesCriteria(bridgeId, ownerGroupId, session);
                return criteria.list();
            }
        };
        return getHibernateTemplate().executeFind(callback);
    }

    public List<Conference> searchConferences(final String searchTerm) {
        String searchTermLike = (new StringBuilder()).append(PERCENT).append(searchTerm).append(PERCENT).toString();
        List<Conference> conferences = new ArrayList<Conference>();
        List<Object[]> results = getHibernateTemplate().findByNamedQueryAndNamedParam("searchConferences",
                new String[] {
                    "name", "ext", "description", "ownerName", "ownerUName"
                }, new String[] {
                    searchTermLike, searchTerm, searchTermLike, searchTermLike, searchTerm
                });
        for (Object[] result : results) {
            for (int i = 0; i < result.length; i++) {
                if (result[i] instanceof Conference) {
                    conferences.add((Conference) result[i]);
                }
            }
        }
        return conferences;
    }

    public int countFilterConferences(final Integer bridgeId, final Integer ownerGroupId) {
        HibernateCallback callback = new HibernateCallback() {
            public Object doInHibernate(Session session) {
                Criteria criteria = filterConferencesCriteria(bridgeId, ownerGroupId, session);
                criteria.setProjection(Projections.rowCount());
                List results = criteria.list();
                return results;
            }
        };
        List list = getHibernateTemplate().executeFind(callback);
        Integer count = (Integer) list.get(0);
        return count.intValue();
    }

    public List<Conference> filterConferencesByPage(final Integer bridgeId, final Integer ownerGroupId,
            final int firstRow, final int pageSize, final String[] orderBy, final boolean orderAscending) {

        HibernateCallback callback = new HibernateCallback() {
            public Object doInHibernate(Session session) {
                Criteria criteria = filterConferencesCriteria(bridgeId, ownerGroupId, session);
                criteria.setFirstResult(firstRow);
                criteria.setMaxResults(pageSize);
                for (int i = 0; i < orderBy.length; i++) {
                    Order order = orderAscending ? Order.asc(orderBy[i]) : Order.desc(orderBy[i]);
                    criteria.addOrder(order);
                }
                return criteria.list();
            }
        };
        return getHibernateTemplate().executeFind(callback);
    }

    public String getAddressSpec(Conference conference) {
        String domain = m_domainManager.getDomain().getName();
        return SipUri.fix(conference.getExtension(), domain);
    }

    public Bridge getBridgeForLocationId(Integer locationId) {
        HibernateTemplate hibernate = getHibernateTemplate();
        List<Bridge> servers = hibernate.findByNamedQueryAndNamedParam("bridgeForLocationId", "locationId",
                locationId);

        return (Bridge) DataAccessUtils.singleResult(servers);
    }

    @Required
    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    @Override
    public void removeBridge(Bridge bridge) {
        getHibernateTemplate().delete(bridge);
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof User) {
            User u = (User) entity;
            removeConferences(findConferencesByOwner(u));
        }
    }

    @Override
    public void onSave(Object entity) {
    }

}
