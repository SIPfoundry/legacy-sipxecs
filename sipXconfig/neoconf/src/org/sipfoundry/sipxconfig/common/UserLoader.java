/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.Query;
import org.hibernate.Session;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;

/** Helper class for loading users by query */
public class UserLoader {
    private static final Log LOG = LogFactory.getLog(UserLoader.class);
    
    // names of query parameters
    private static final String PARAM_GROUP_ID = "groupId";
    private static final String PARAM_SEARCH = "search";
    private static final String PARAM_VALUE = "value";

    // strings for SQL aliases (not sipX aliases!)
    private static final String ALIASES_ALIAS = "alias";
    private static final String USER_ALIAS = "u";
    
    // checkstyle wants you to name your string literals
    private static final String DOT = ".";
    private static final String SPACE = " ";
    private static final String WHERE = "where";
    
    private Session m_session;
    private StringBuffer m_queryBuf;
    private boolean m_noWhere;      // true if we haven't added the SQL "where" keyword yet
    private List m_paramNames = new ArrayList();
    private List m_paramValues = new ArrayList();
    private boolean m_outerJoin;    // true if we are using an outer join

    /** 
     * Create a UserLoader with a Session.
     * We shouldn't hold onto Sessions for very long, similarly let go of the UserLoader
     * as soon as you are done with the task at hand, don't keep it around for reuse.
     */
    public UserLoader(Session session) {
        m_session = session;
    }
    
    public List loadUsersByPage(String search, Integer groupId, int firstRow, int pageSize, String orderBy,
            boolean orderAscending) {
        // create the query
        Query query = createUserQuery(search, groupId, orderBy, orderAscending, false);
        
        // execute the query and return results
        List users = queryUsersByPage(query, firstRow, pageSize);
        return users;
    }
    
    /**
     * Count users who match the search string and are in the group.
     * TODO: This is brutally inefficient.  Because of the outer join required to query aliases,
     * the query may return duplicates, so we can't do a simple SQL count, we have to pull data
     * back and filter out the duplicates.  Maybe there is a better way to do this?
     * At least the search constraint will reduce the data size.  In fact if there is no search
     * constraint, we'll throw an exception to force you to use CoreContext.getUsersInGroupCount
     * instead.
     * Return Integer rather than int because we're operating inside a Hibernate callback and
     * must return an Object.
     */
    public Integer countUsers(String search, Integer groupId) {
        if (StringUtils.isBlank(search)) {
            throw new IllegalArgumentException("Search string must not be empty");
        }
        
        // create the query
        Query query = createUserQuery(search, groupId, null, true, false);
        
        // execute it & get a bunch of IDs
        List ids = query.list();
        
        // count them, excluding duplicates
        HashSet idSet = new HashSet(ids);
        return new Integer(idSet.size());
    }

    // Create and return the user query.
    // If getUserIdsOnly is true, then get just the user IDs, not the users.
    private Query createUserQuery(String search, Integer groupId, String orderBy,
            boolean orderAscending, boolean getUserIdsOnly) {
        init(getUserIdsOnly);
        
        // add constraints
        handleSearchConstraint(search);
        handleGroupConstraint(groupId);

        // sort the results
        m_queryBuf.append("order by " + USER_ALIAS + DOT);
        m_queryBuf.append(StringUtils.defaultIfEmpty(orderBy, User.LAST_NAME_PROP));
        m_queryBuf.append(SPACE);
        m_queryBuf.append(orderAscending ? "asc" : "desc");
                        
        // create the query and add parameters
        Query query = m_session.createQuery(m_queryBuf.toString());
        Iterator iter1 = m_paramNames.iterator();
        for (Iterator iter2 = m_paramValues.iterator(); iter2.hasNext();) {
            String name = (String) iter1.next();
            String value = (String) iter2.next();
            query.setString(name, value);
        }
        
        return query;
    }
    
    private void handleSearchConstraint(String search) {
        if (!StringUtils.isEmpty(search)) {
            // append "left outer join u.aliases alias "
            m_queryBuf.append("left outer join " + USER_ALIAS + DOT + User.ALIASES_PROP
                    + SPACE + ALIASES_ALIAS + SPACE);
            m_outerJoin = true;
            
            // start the constraint with "where "
            addWhere();
            
            // query userName
            // append "( lower(u.userName) like :search"
            final String likeSearch = ") like :" + PARAM_SEARCH;
            final String lower = "lower(";
            final String or = " or ";
            m_queryBuf.append("(" + lower + USER_ALIAS + DOT + User.USER_NAME_PROP + likeSearch);
            
            // query aliases
            // append " or lower(alias) like :search "
            m_queryBuf.append(or + lower + ALIASES_ALIAS + likeSearch);
            
            // query firstName
            // append " or lower(u.firstName) like :search "
            m_queryBuf.append(or + lower + USER_ALIAS + DOT + User.FIRST_NAME_PROP + likeSearch);
            
            // query lastName and close paren
            // append " or lower(u.lastName) like :search "
            m_queryBuf.append(or + lower + USER_ALIAS + DOT + User.LAST_NAME_PROP + likeSearch + ")"
                    + SPACE);
            
            m_paramNames.add(PARAM_SEARCH);
            addWildParamValue_(search.toLowerCase());
        }
    }
    
    private void handleGroupConstraint(Integer groupId) {
        if (groupId != null) {
            boolean addedWhere = addWhere();
            
            // If we are piggybacking on an existing "where" clause then use "and" 
            // to combine this constraint with the previous one
            if (!addedWhere) {
                m_queryBuf.append(" and ");
            }
            
            // append "u.groups.id = :groupId"
            m_queryBuf.append(USER_ALIAS + DOT + BeanWithGroups.GROUPS_PROP + DOT + "id = :"
                    + PARAM_GROUP_ID + SPACE);
            
            m_paramNames.add(PARAM_GROUP_ID);
            addParamValue_(groupId.toString());            
        }
    }

    private List queryUsersByPage(Query query, int firstRow, int pageSize) {
        List users = null;
        if (m_outerJoin) {
            // Execute the query.  Eliminate any duplicates in the users list.  Because of
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
     * Add "where " to begin the constraint clause, if it hasn't already been added.
     * Return true if we actually added it, false if it was there already.
     */
    private boolean addWhere() {
        boolean addedWhere = m_noWhere;
        if (m_noWhere) {
            m_queryBuf.append(WHERE + SPACE);
            m_noWhere = false;     
        }
        return addedWhere;
    }
    
    // Load users that match userTemplate, a user example.
    // This method will be retired in favor of loadUsersByPage when possible.
    // That's why there is code duplication across these two routines, this one is going away.
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
        Iterator iter1 = m_paramNames.iterator();
        for (Iterator iter2 = m_paramValues.iterator(); iter2.hasNext();) {
            String name = (String) iter1.next();
            String value = (String) iter2.next();
            query.setString(name, value);
        }
        
        // Execute the query.  Eliminate any duplicates in the users list.
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
            
            m_paramNames.add(PARAM_VALUE);
            addWildParamValue_(userTemplate.getUserName());
            m_noWhere = false;  // we added "where" to the query string
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

            m_paramNames.add(User.FIRST_NAME_PROP);
            addWildParamValue_(userTemplate.getFirstName());
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

            m_paramNames.add(User.LAST_NAME_PROP);
            addWildParamValue_(userTemplate.getLastName());
        }
    }

    /**
     * Add a param value to the list of values.
     * Put a SQL wildcard "%" at both the beginning and end of the value string.  
     */
    // Put an underscore at the end of the method name to suppress a bogus
    // warning from Checkstyle about this method being unused.
    private void addWildParamValue_(String value) {
        final String wild = "%";
        m_paramValues.add(wild + value + wild);
    }

    /**
     * Add a param value to the list of values.
     */
    // Put an underscore at the end of the method name to suppress a bogus
    // warning from Checkstyle about this method being unused.
    private void addParamValue_(String value) {
        m_paramValues.add(value);
    }

    /** 
     * For the query contraint, add SQL "where " if this is the first constraint
     * or "and " if this is a subsequent constraint.
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
        String beginQuery = "";
        if (getUserIdsOnly) {
            beginQuery = "select " + USER_ALIAS + DOT + BeanWithId.ID_PROPERTY + SPACE;
        }
        m_queryBuf = new StringBuffer(beginQuery + "from User " + USER_ALIAS + SPACE);
        m_noWhere = true;
        m_paramNames.clear();
        m_paramValues.clear();
        m_outerJoin = false;
    }
    
    /**
     * Remove duplicates from the users list and return the new list.
     */
    private List removeDuplicateUsers(List users) {        
        // Store each user in a map, indexed by userName and look for collisions.
        // userName is guaranteed to be unique.
        List uniqueUsers = new ArrayList(users.size());
        Map usersMap = new HashMap();
        for (Iterator iter = users.iterator(); iter.hasNext();) {
            User user = (User) iter.next();
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
