/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.common;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.Query;
import org.hibernate.Session;

/** Helper class for loading users by query */
public class UserLoader {
    private static final Log LOG = LogFactory.getLog(UserLoader.class);

    private static final String PARAM_VALUE = "value";
    private static final String AND = " and ";

    // checkstyle wants you to name your string literals
    private static final String SPACE = " ";
    private static final String WHERE = "where";

    private final Session m_session;
    private StringBuilder m_queryBuf;
    private boolean m_noWhere; // true if we haven't added the SQL "where" keyword yet
    private final Map<String, Object> m_params = new LinkedHashMap<String, Object>();
    private boolean m_outerJoin; // true if we are using an outer join

    /**
     * Create a UserLoader with a Session. We shouldn't hold onto Sessions for very long,
     * similarly let go of the UserLoader as soon as you are done with the task at hand, don't
     * keep it around for reuse.
     */
    public UserLoader(Session session) {
        m_session = session;
    }

    public List loadUsersByPage(String search, Integer groupId, Integer branchId, int firstRow, int pageSize,
            String orderBy, boolean orderAscending) {
        // create the query
        Query query = createUserQuery(search, groupId, branchId, orderBy, orderAscending, false);

        // execute the query and return results
        List users = queryUsersByPage(query, firstRow, pageSize);
        return users;
    }

    /**
     * Count users who match the search string and are in the group.
     *
     * TODO: This is brutally inefficient. Because of the outer join required to query aliases,
     * the query may return duplicates, so we can't do a simple SQL count, we have to pull data
     * back and filter out the duplicates. Maybe there is a better way to do this? At least the
     * search constraint will reduce the data size. In fact if there is no search constraint,
     * we'll throw an exception to force you to use CoreContext.getUsersInGroupCount instead.
     * Return Integer rather than int because we're operating inside a Hibernate callback and must
     * return an Object.
     */
    public Integer countUsers(String search, Integer groupId) {
        if (StringUtils.isBlank(search)) {
            throw new IllegalArgumentException("Search string must not be empty");
        }

        // create the query
        Query query = createUserQuery(search, groupId, null, null, true, false);

        // execute it & get a bunch of IDs
        List ids = query.list();

        // count them, excluding duplicates
        HashSet idSet = new HashSet(ids);
        return idSet.size();
    }

    // Create and return the user query.
    // If getUserIdsOnly is true, then get just the user IDs, not the users.
    private Query createUserQuery(String search, Integer groupId, Integer branchId, String orderBy,
            boolean orderAscending, boolean getUserIdsOnly) {
        init(getUserIdsOnly);

        // add constraints
        handleSearchConstraint(search, groupId);
        handleGroupConstraint(groupId);
        handleBranchConstraint(branchId);

        // sort the results
        m_queryBuf.append(" order by u.");
        m_queryBuf.append(StringUtils.defaultIfEmpty(orderBy, "lastName"));
        m_queryBuf.append(orderAscending ? " asc " : " desc ");

        // create the query and add parameters
        Query query = m_session.createQuery(m_queryBuf.toString());
        addParams(query);

        return query;
    }

    private void addParams(Query query) {
        for (Map.Entry<String, Object> e : m_params.entrySet()) {
            String name = e.getKey();
            Object value = e.getValue();
            if (value instanceof Integer) {
                Integer valueInt = (Integer) value;
                query.setInteger(name, valueInt);
            }
            if (value instanceof String) {
                String valueStr = (String) value;
                query.setString(name, valueStr);
            }
        }
    }

    private void handleSearchConstraint(String search, Integer groupId) {
        if (groupId != null) {
            m_queryBuf.append(" join u.groups ugroups ");
        }

        if (!StringUtils.isEmpty(search)) {
            m_queryBuf.append(" left outer join u.aliases alias ");
            m_outerJoin = true;

            addWhere();

            m_queryBuf.append("(lower(u.userName) like :search ");
            m_queryBuf.append(" or lower(alias) like :search");
            m_queryBuf.append(" or lower(u.firstName) like :search");
            m_queryBuf.append(" or lower(u.lastName) like :search) ");

            m_params.put("search", wildcard(search));
        }
    }

    private void handleGroupConstraint(Integer groupId) {
        if (groupId != null) {
            boolean addedWhere = addWhere();

            // If we are piggybacking on an existing "where" clause then use "and"
            // to combine this constraint with the previous one
            if (!addedWhere) {
                m_queryBuf.append(AND);
            }

            m_queryBuf.append(" ugroups.id = :groupId ");
            m_params.put("groupId", groupId);
        }
    }

    private void handleBranchConstraint(Integer branchId) {
        if (branchId != null) {
            boolean addedWhere = addWhere();

            // If we are piggybacking on an existing "where" clause then use "and"
            // to combine this constraint with the previous one
            if (!addedWhere) {
                m_queryBuf.append(AND);
            }

            m_queryBuf.append(" u.branch.id = :branchId ");
            m_params.put("branchId", branchId);
        }
    }

    private List queryUsersByPage(Query query, int firstRow, int pageSize) {
        List users = null;
        if (m_outerJoin) {
            // Execute the query. Eliminate any duplicates in the users list. Because of
            // duplicates, we can't use standard pagination and have to paginate manually.
            // See http://www.hibernate.org/117.html#A11 -- the "distinct"
            // keyword in HQL won't remove duplicates, in the case of an outer join.
            users = query.list();
            users = removeDuplicateUsers(users);
            int numUsers = users.size();
            if (firstRow < numUsers) {
                users = users.subList(firstRow, Math.min(firstRow + pageSize, numUsers));
            } else {
                users.clear();
            }
        } else {
            // The query does not include an outer join, so we can use standard pagination.
            query.setFirstResult(firstRow);
            query.setMaxResults(pageSize);
            users = query.list();
        }
        return users;
    }

    /**
     * Add "where " to begin the constraint clause, if it hasn't already been added. Return true
     * if we actually added it, false if it was there already.
     */
    private boolean addWhere() {
        boolean addedWhere = m_noWhere;
        if (m_noWhere) {
            m_queryBuf.append(WHERE + SPACE);
            m_noWhere = false;
        }
        return addedWhere;
    }

    public List loadUsers(final User userTemplate) {
        init(false);

        // Add constraints
        handleUserNameAliasesConstraint(userTemplate);
        handleFirstNameConstraint(userTemplate);
        handleLastNameConstraint(userTemplate);

        // Sort by last name
        m_queryBuf.append("order by u.lastName asc");

        // Create the query and add parameters
        Query query = m_session.createQuery(m_queryBuf.toString());
        addParams(query);

        // Execute the query. Eliminate any duplicates in the users list.
        // See http://www.hibernate.org/117.html#A11 -- we can't count on the "distinct"
        // keyword in HQL to remove duplicates, because the query may use an outer join.
        List users = query.list();
        users = removeDuplicateUsers(users);

        return users;
    }

    /**
     * Handle userName/aliases constraint
     */
    private void handleUserNameAliasesConstraint(final User userTemplate) {
        if (!StringUtils.isEmpty(userTemplate.getUserName())) {
            m_queryBuf.append("left outer join u.aliases alias ");
            m_queryBuf.append("where (u.userName like :");
            m_queryBuf.append(PARAM_VALUE);
            m_queryBuf.append("  or alias like :");
            m_queryBuf.append(PARAM_VALUE);
            m_queryBuf.append(" ) ");

            m_params.put(PARAM_VALUE, wildcard(userTemplate.getUserName()));
            m_noWhere = false; // we added "where" to the query string
        }
    }

    /**
     * Handle firstName constraint
     */
    private void handleFirstNameConstraint(final User userTemplate) {
        if (!StringUtils.isEmpty(userTemplate.getFirstName())) {
            startQueryConstraint();
            m_queryBuf.append("u.firstName like :");
            m_queryBuf.append(User.FIRST_NAME_PROP);
            m_queryBuf.append(SPACE);

            m_params.put(User.FIRST_NAME_PROP, wildcard(userTemplate.getFirstName()));
        }
    }

    /**
     * Handle lastName constraint
     */
    private void handleLastNameConstraint(final User userTemplate) {
        if (!StringUtils.isEmpty(userTemplate.getLastName())) {
            startQueryConstraint();
            m_queryBuf.append("u.lastName like :");
            m_queryBuf.append(User.LAST_NAME_PROP);
            m_queryBuf.append(SPACE);

            m_params.put(User.LAST_NAME_PROP, wildcard(userTemplate.getLastName()));
        }
    }

    /**
     * Put a SQL wildcard "%" at both the beginning and end of the value string.
     */
    private String wildcard(String value) {
        final String wild = "%";
        return wild + value.toLowerCase() + wild;
    }

    /**
     * For the query constraint, add SQL "where " if this is the first constraint or "and " if
     * this is a subsequent constraint.
     */
    private void startQueryConstraint() {
        if (m_noWhere) {
            m_queryBuf.append("where ");
            m_noWhere = false;
        } else {
            m_queryBuf.append("and ");
        }
    }

    /** Initialize internal state to get ready for a new query */
    private void init(boolean getUserIdsOnly) {
        m_queryBuf = new StringBuilder();
        if (getUserIdsOnly) {
            m_queryBuf.append("select distinct u.id ");
        } else {
            m_queryBuf.append("select distinct u ");
        }
        m_queryBuf.append(" from User u ");
        m_noWhere = true;
        m_params.clear();
        m_outerJoin = false;
    }

    /**
     * Remove duplicates from the users list and return the new list.
     */
    private List removeDuplicateUsers(List<User> users) {
        // Store each user in a map, indexed by userName and look for collisions.
        // userName is guaranteed to be unique.
        List uniqueUsers = new ArrayList(users.size());
        Map usersMap = new HashMap();
        for (User user : users) {
            if (!usersMap.containsKey(user.getId())) {
                usersMap.put(user.getId(), null);
                uniqueUsers.add(user);
            } else {
                if (LOG.isDebugEnabled()) {
                    LOG.debug("Skipping duplicate user " + user);
                }
            }
        }
        return uniqueUsers;
    }
}
