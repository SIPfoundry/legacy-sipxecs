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
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.apache.commons.lang.RandomStringUtils;
import org.apache.commons.lang.StringUtils;
import org.hibernate.Criteria;
import org.hibernate.Hibernate;
import org.hibernate.Query;
import org.hibernate.Session;
import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.commons.userdb.profile.Address;
import org.sipfoundry.commons.userdb.profile.UserProfileService;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.orm.hibernate3.HibernateCallback;

public abstract class CoreContextImpl extends SipxHibernateDaoSupport<User> implements CoreContext,
       ApplicationContextAware, SetupListener {

    public static final String ADMIN_GROUP_NAME = "administrators";
    public static final String CONTEXT_BEAN_NAME = "coreContextImpl";
    private static final int SIP_PASSWORD_LEN = 12;
    private static final String USERNAME_PROP_NAME = "userName";
    private static final String VALUE = "value";
    /** nothing special about this name */
    private static final String QUERY_USER_BY_NAME_OR_ALIAS = "userByNameOrAlias";
    private static final String SQL_QUERY_USER_IDS_BY_NAME_OR_ALIAS = "select distinct u.user_id from users u "
            + "left outer join user_alias alias  " + "on u.user_id=alias.user_id  "
            + "left outer join value_storage vs on vs.value_storage_id=u.value_storage_id  "
            + "left outer join setting_value sv on sv.value_storage_id=vs.value_storage_id  "
            + "where u.user_name= :alias or alias.alias= :alias  "
            + "or (sv.path='voicemail/fax/did' and sv.value= :alias)  "
            + "or (sv.path='voicemail/fax/extension' and sv.value= :alias) ";
    private static final String SQL_QUERY_USER_IDS_BY_NAME_OR_ALIAS_EXCEPT_THIS =
        "select distinct u.user_id from users "
        + "u left outer join user_alias alias  "
        + "on u.user_id=alias.user_id left "
        + "outer join value_storage vs on vs.value_storage_id=u.value_storage_id left "
        + "outer join setting_value sv on sv.value_storage_id=vs.value_storage_id  "
        + "where (u.user_name= :alias or alias.alias= :alias  "
        + "or (sv.path='voicemail/fax/did' and sv.value = :alias)  "
        + "or (sv.path='voicemail/fax/extension' and sv.value = :alias)) and u.user_name != :username";
    private static final String ALIAS = "alias";
    private static final String QUERY_USER = "from AbstractUser";
    private static final String QUERY_PARAM_GROUP_ID = "groupId";
    private static final String QUERY_IM_ID = "imId";
    private static final String QUERY_USER_ID = "userId";
    private static final String SPECIAL_USER_BY_TYPE = "specialUserByType";
    private static final String SPECIAL_USER_TYPE = "specialUserType";
    private static final String USER_ADMIN = "userAdmin";
    private static final String FIRST = "first";
    private static final String PAGE_SIZE = "pageSize";
    private static final String USER_ID = "user_id";

    private DomainManager m_domainManager;
    private SettingDao m_settingDao;
    private AliasManager m_aliasManager;
    private ApplicationContext m_applicationContext;
    private JdbcTemplate m_jdbcTemplate;
    private boolean m_debug;
    private boolean m_setup;

    /** limit number of users */
    private int m_maxUserCount = -1;

    public CoreContextImpl() {
        super();
    }

    /**
     * Implemented by Spring lookup-method injection
     */
    @Override
    public abstract User newUser();

    /**
     * Implemented by Spring lookup-method injection
     */
    @Override
    public abstract InternalUser newInternalUser();

    public abstract UserProfileService getUserProfileService();

    @Override
    public boolean getDebug() {
        return m_debug;
    }

    public void setDebug(boolean debug) {
        m_debug = debug;
    }

    @Override
    public String getAuthorizationRealm() {
        return m_domainManager.getAuthorizationRealm();
    }

    public void setMaxUserCount(int maxUserCount) {
        m_maxUserCount = maxUserCount;
    }

    @Override
    public String getDomainName() {
        return m_domainManager.getDomain().getName();
    }

    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }

    @Override
    public boolean saveUser(User user) {
        boolean newUserName = user.isNew();
        String dup = null;
        try {
            dup = checkForDuplicateNameOrAlias(user);
        } catch (Exception ex) {
            throw new UserException("&err.msg.checkUserIdAliasFailed", ex);
        }
        if (dup != null) {
            throw new NameInUseException(dup);
        }

        checkImIdUnique(user);
        checkMaxUsers(user, m_maxUserCount);
        checkBranch(user);
        String origUserName = null;
        if (!user.isNew()) {
            origUserName = (String) getOriginalValue(user, USERNAME_PROP_NAME);
            if (!origUserName.equals(user.getUserName())) {
                if (origUserName.equals(User.SUPERADMIN)) {
                    throw new UserException("&msg.error.renameAdminUser");
                }
                newUserName = true;
                String origPintoken = (String) getOriginalValue(user, "pintoken");
                if (origPintoken.equals(user.getPintoken())) {
                    throw new ChangePintokenRequiredException("&error.changePintokenRequiredException");
                }
            }
        } else {
            user.getUserProfile().setUseBranchAddress(true);
        }

        if (user.getUserProfile().getUseBranchAddress() && user.getSite() != null) {
            Address branch = user.getUserProfile().getBranchAddress();
            if (branch == null) {
                branch = new Address();
                user.getUserProfile().setBranchAddress(branch);
            }
            branch.setCity(user.getSite().getAddress().getCity());
            branch.setCountry(user.getSite().getAddress().getCountry());
            branch.setOfficeDesignation(user.getSite().getAddress().getOfficeDesignation());
            branch.setState(user.getSite().getAddress().getState());
            branch.setStreet(user.getSite().getAddress().getStreet());
            branch.setZip(user.getSite().getAddress().getZip());
            user.getUserProfile().setBranchName(user.getSite().getName());
        }
        if (!user.isNew() && user.getSite() == null) {
            user.getUserProfile().setUseBranchAddress(false);
            user.getUserProfile().setBranchAddress(new Address());
        }
        if (user.isNew()) {
            getHibernateTemplate().save(user);
        } else {
            getHibernateTemplate().merge(user);
        }
        return newUserName;
    }

    @Override
    public String getOriginalUserName(User user) {
        return (String) getOriginalValue(user, USERNAME_PROP_NAME);
    }

    /**
     * Check that the system has been restricted to a certain number of users
     *
     * @param maxUserCount -1 or represent infinite number
     */
    void checkMaxUsers(User user, int maxUserCount) {
        // allow edits to the Nth (or beyond) user
        if (!user.isNew()) {
            return;
        }

        if (maxUserCount < 0) {
            return;
        }

        int count = getUsersCount();
        if (count >= maxUserCount) {
            throw new MaxUsersException(m_maxUserCount);
        }
    }

    static class MaxUsersException extends UserException {
        MaxUsersException(int maxCount) {
            super("You cannot exceed the maximum number of allowed users: " + maxCount);
        }
    }

    public static class ChangePintokenRequiredException extends UserException {
        public ChangePintokenRequiredException(String msg) {
            super(msg);
        }
    }

    @Override
    public void deleteUser(User user) {
        getHibernateTemplate().delete(user);
    }

    @Override
    public boolean deleteUsers(Collection<Integer> userIds) {
        if (userIds.isEmpty()) {
            // no users to delete => nothing to do
            return false;
        }

        User admin = loadUserByUserName(User.SUPERADMIN);
        boolean affectAdmin = false;
        List<User> users = new ArrayList<User>(userIds.size());
        for (Integer id : userIds) {
            User user = loadUser(id);
            if (user != admin) {
                users.add(user);
            } else {
                affectAdmin = true;
            }
        }
        getDaoEventPublisher().publishDeleteCollection(users);
        getHibernateTemplate().deleteAll(users);
        return affectAdmin;
    }

    @Override
    public void deleteUsersByUserName(Collection<String> userNames) {
        if (userNames.isEmpty()) {
            // no users to delete => nothing to do
            return;
        }
        List users = new ArrayList(userNames.size());
        for (String userName : userNames) {
            User user = loadUserByUserName(userName);
            users.add(user);
        }
        getDaoEventPublisher().publishDeleteCollection(users);
        getHibernateTemplate().deleteAll(users);
    }

    @Override
    public User loadUser(Integer id) {
        return load(User.class, id);
    }

    @Override
    public User getUser(Integer id) {
        return getHibernateTemplate().get(User.class, id);
    }

    @Override
    public User loadUserByUserName(String userName) {
        return loadUserByNamedQueryAndNamedParam("userByUserName", VALUE, userName);
    }

    private User loadUserByUniqueProperty(String propName, String propValue) {
        final Criterion expression = Restrictions.eq(propName, propValue);

        HibernateCallback callback = new HibernateCallback() {
            @Override
            public Object doInHibernate(Session session) {
                Criteria criteria = session.createCriteria(User.class).add(expression);
                return criteria.list();
            }
        };
        List users = getHibernateTemplate().executeFind(callback);
        User user = (User) DaoUtils.requireOneOrZero(users, expression.toString());

        return user;
    }

    @Override
    public User loadUserByAlias(String alias) {
        return loadUserByNamedQueryAndNamedParam("userByAlias", VALUE, alias);
    }

    @Override
    public User loadUserByConfiguredImId(String imId) {
        Integer userId = getUserProfileService().getUserIdByImId(imId);
        if (userId != null) {
            return loadUser(userId);
        }
        return null;
    }

    @Override
    public User loadUserByUserNameOrAlias(String userNameOrAlias) {
        return loadUserByNamedQueryAndNamedParam(QUERY_USER_BY_NAME_OR_ALIAS, VALUE, userNameOrAlias);
    }

    @Override
    public List<User> loadUserByAdmin() {
        return getHibernateTemplate().findByNamedQuery(USER_ADMIN);
    }

    private void checkImIdUnique(User user) {
        if (!isImIdUnique(user)) {
            ImAccount accountToSave = new ImAccount(user);
            throw new UserException("&duplicate.imid.error", accountToSave.getImId());
        }
    }

    /**
     * Checks if the inherited branch is the same with the actual branch when they are not null
     *
     * @param user
     */
    private void checkBranch(User user) {
        Branch inheritedBranch = user.getInheritedBranch();
        Branch branch = user.getBranch();
        if (inheritedBranch != null && branch != null
                && !StringUtils.equals(inheritedBranch.getName(), branch.getName())) {
            throw new UserException("&invalid.branch");
        }
    }

    @Override
    public Collection<User> getUsersForBranch(Branch branch) {
        Collection<User> users = getHibernateTemplate().findByNamedQueryAndNamedParam("usersForBranch", "branch",
                branch);
        return users;
    }

    /**
     * Check whether the user has a username or alias or ImId that collides with an existing
     * username or alias. Check for internal collisions as well, for example, the user has an
     * alias that is the same as the username. (Duplication within the aliases is not possible
     * because the aliases are stored as a Set.) If there is a collision, then return the bad name
     * (username or alias). Otherwise return null. If there are multiple collisions, then it's
     * arbitrary which name is returned.
     *
     * @param user user to test
     * @return name that collides
     */
    @Override
    public String checkForDuplicateNameOrAlias(User user) {
        String result = null;

        // Check for duplication within the user itself
        List names = new ArrayList(user.getAliases());
        String userName = user.getUserName();
        names.add(userName);
        String faxExtension = user.getFaxExtension();
        if (!faxExtension.isEmpty()) {
            names.add(faxExtension);
        }
        String faxDid = user.getFaxDid();
        if (!faxDid.isEmpty()) {
            names.add(faxDid);
        }
        result = checkForDuplicateString(names);
        if (result == null) {
            // Check whether the userName is a duplicate.
            if (!m_aliasManager.canObjectUseAlias(user, userName)) {
                result = userName;
            } else {
                // Check the aliases and return any duplicate as a bad name.
                for (String alias : user.getAliases()) {
                    if (!m_aliasManager.canObjectUseAlias(user, alias)) {
                        result = alias;
                        break;
                    }
                }
                // check if user ImId is unique in alias namespace
                ImAccount imAccount = new ImAccount(user);
                if (!m_aliasManager.canObjectUseAlias(user, imAccount.getImId())) {
                    result = imAccount.getImId();
                }

                // check if the user's fax extension and DID areunique in the alias namespace
                if (!faxExtension.isEmpty()) {
                    if (!m_aliasManager.canObjectUseAlias(user, faxExtension)) {
                        result = faxExtension;
                    }
                }
                if (!faxDid.isEmpty()) {
                    if (!m_aliasManager.canObjectUseAlias(user, faxDid)) {
                        result = faxDid;
                    }
                }
            }
        }

        return result;
    }

    /**
     * Given a collection of strings, look for duplicates. Return the first duplicate found, or
     * null if all strings are unique.
     */
    String checkForDuplicateString(Collection<String> strings) {
        Set<String> set = new TreeSet<String>();
        for (String str : strings) {
            if (!set.add(str)) {
                return str;
            }
        }
        return null;
    }

    private User loadUserByNamedQueryAndNamedParam(String queryName, String paramName, Object value) {
        Collection usersColl = getHibernateTemplate().findByNamedQueryAndNamedParam(queryName, paramName, value);
        Set users = new HashSet(usersColl); // eliminate duplicates
        if (users.size() > 1) {
            throw new IllegalStateException("The database has more than one user matching the query " + queryName
                    + ", paramName = " + paramName + ", value = " + value);
        }
        User user = null;
        if (users.size() > 0) {
            user = (User) users.iterator().next();
        }
        return user;
    }

    /**
     * Return all users matching the userTemplate example. Empty properties of userTemplate are
     * ignored in the search. The userName property matches either the userName or aliases
     * properties.
     */
    @Override
    public List<User> loadUserByTemplateUser(final User userTemplate) {
        HibernateCallback callback = new HibernateCallback() {
            @Override
            public Object doInHibernate(Session session) {
                UserLoader loader = new UserLoader(session);
                return loader.loadUsers(userTemplate);
            }
        };
        List<User> users = getHibernateTemplate().executeFind(callback);
        return users;
    }

    @Override
    public List<User> loadUsers() {
        return getHibernateTemplate().loadAll(User.class);
    }

    @Override
    public int getUsersCount() {
        return getUsersInGroupCount(null);
    }

    // returns only the number of users created by admin
    @Override
    public int getAllUsersCount() {
        return getBeansInGroupCount(AbstractUser.class, null);
    }

    @Override
    public int getUsersInGroupCount(Integer groupId) {
        return getBeansInGroupCount(User.class, groupId);
    }

    @Override
    public int getUsersInGroupWithSearchCount(final Integer groupId, final String searchString) {
        int numUsers = 0;
        if (!StringUtils.isEmpty(searchString)) {
            HibernateCallback callback = new HibernateCallback() {
                @Override
                public Object doInHibernate(Session session) {
                    UserLoader loader = new UserLoader(session);
                    return loader.countUsers(searchString, groupId);
                }
            };
            Integer count = (Integer) getHibernateTemplate().execute(callback);
            numUsers = count.intValue();
        } else {
            numUsers = getUsersInGroupCount(groupId);
        }
        return numUsers;
    }

    @Override
    public List<User> getSharedUsers() {
        Collection sharedUsers = getHibernateTemplate().findByNamedQueryAndNamedParam("sharedUsers", "isShared",
                true);
        return new ArrayList<User>(sharedUsers);
    }

    @Override
    public List<User> loadUsersByPage(final String search, final Integer groupId, final Integer branchId,
            final int firstRow, final int pageSize, final String orderBy, final boolean orderAscending) {
        HibernateCallback callback = new HibernateCallback() {
            @Override
            public Object doInHibernate(Session session) {
                UserLoader loader = new UserLoader(session);
                return loader
                        .loadUsersByPage(search, groupId, branchId, firstRow, pageSize, orderBy, orderAscending);
            }
        };
        List<User> users = getHibernateTemplate().executeFind(callback);
        return users;
    }

    @Override
    public List<User> loadUsersByPage(int first, int pageSize) {
        Query q = getHibernateTemplate().getSessionFactory().getCurrentSession()
                .createSQLQuery("select * from users where user_type='C' "
                + "order by user_id limit :pageSize offset :first")
                .addEntity(User.class);
        q.setInteger(FIRST, first);
        q.setInteger(PAGE_SIZE, pageSize);
        List<User> users = q.list();
        return users;
    }

    @Override
    public List<Integer> loadUserIdsByPage(int first, int pageSize) {
        Query q = getHibernateTemplate().getSessionFactory().getCurrentSession()
                .createSQLQuery("select user_id from users where user_type='C' order"
                    + " by user_id limit :pageSize offset :first")
                .addScalar(USER_ID, Hibernate.INTEGER);
        q.setInteger(FIRST, first);
        q.setInteger(PAGE_SIZE, pageSize);
        List<Integer> users = q.list();
        return users;
    }

    @Override
    public List<InternalUser> loadInternalUsers() {
        return getHibernateTemplate().loadAll(InternalUser.class);
    }

    @Override
    public void clear() {
        Collection c = getHibernateTemplate().find(QUERY_USER);
        getHibernateTemplate().deleteAll(c);
    }

    /**
     * Create a superadmin user with an empty pin. This is used to recover from the loss of all
     * users from the database.
     */
    @Override
    public void createAdminGroupAndInitialUserTask() {
        createAdminGroupAndInitialUser(null);
    }

    /**
     * Create a superadmin user with the specified pin.
     *
     * Map an empty pin to an empty pintoken as a special hack allowing the empty pin to be used
     * when an insecure, easy to remember pin is needed. Previously we used 'password' rather than
     * the empty string, relying on another hack that allowed the password and pintoken to be the
     * same. That hack is gone so setting the pintoken to 'password' would no longer work because
     * the password would then be the inverse hash of 'password' rather than 'password'.
     */
    @Override
    public void createAdminGroupAndInitialUser(String pin) {
        Group adminGroup = m_settingDao.getGroupByName(User.GROUP_RESOURCE_ID, ADMIN_GROUP_NAME);
        if (adminGroup == null) {
            adminGroup = new Group();
            adminGroup.setName(ADMIN_GROUP_NAME);
            adminGroup.setResource(User.GROUP_RESOURCE_ID);
            adminGroup.setDescription("Users with superadmin privileges");
        }
        PermissionName.SUPERADMIN.setEnabled(adminGroup, true);
        PermissionName.TUI_CHANGE_PIN.setEnabled(adminGroup, false);

        m_settingDao.saveGroup(adminGroup);

        User admin = loadUserByUserName(User.SUPERADMIN);
        if (admin == null) {
            admin = newUser();
            admin.setUserName(User.SUPERADMIN);

            // currently superadmin cannot invite to a conference without a valid sip password
            admin.setSipPassword(RandomStringUtils.randomAlphanumeric(SIP_PASSWORD_LEN));
        } else {
            // if superadmin user already exists make sure it has superadmin permission
            admin.setPermission(PermissionName.SUPERADMIN, true);
        }

        admin.setPin(StringUtils.defaultString(pin));
        admin.addGroup(adminGroup);
        saveUser(admin);
        getDaoEventPublisher().publishSave(admin);
    }

/*    @Override
    public void saveUserToAgentsGroup(User user) {
        createAgentsGroup();
        Group allAgentsGroup = m_settingDao.getGroupByName(User.GROUP_RESOURCE_ID, AGENT_GROUP_NAME);
        if (allAgentsGroup != null) {
            user.getGroups().add(allAgentsGroup);
            getHibernateTemplate().merge(user);
        }
    }

    @Override
    public void saveRemoveUserFromAgentGroup(User user) {
        Group agentGroup = m_settingDao.getGroupByName(User.GROUP_RESOURCE_ID, AGENT_GROUP_NAME);
        if (agentGroup != null) {
            user.removeGroup(agentGroup);
            saveUser(user);
        }
    }*/

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    @Override
    public List<Group> getGroups() {
        return m_settingDao.getGroups(USER_GROUP_RESOURCE_ID);
    }

    @Override
    public List<Group> getAvailableGroups(User user) {
        List<Group> allGroups = getGroups();
        List<Group> availableGroups = new ArrayList<Group>();
        for (Group group : allGroups) {
            if (user.isGroupAvailable(group)) {
                availableGroups.add(group);
            }
        }
        return availableGroups;
    }

    @Override
    public Group getGroupById(Integer groupId) {
        List<Group> groups = m_settingDao.getGroups(USER_GROUP_RESOURCE_ID);
        for (Group group : groups) {
            int id = group.getId();
            if (groupId == id) {
                return group;
            }
        }
        return null;
    }

    @Override
    public Group getGroupByName(String userGroupName, boolean createIfNotFound) {
        if (createIfNotFound) {
            return m_settingDao.getGroupCreateIfNotFound(USER_GROUP_RESOURCE_ID, userGroupName);
        }
        return m_settingDao.getGroupByName(USER_GROUP_RESOURCE_ID, userGroupName);
    }

    @Override
    public Collection<User> getGroupMembers(Group group) {
        Collection<User> users = getHibernateTemplate().findByNamedQueryAndNamedParam("userGroupMembers",
                QUERY_PARAM_GROUP_ID, group.getId());
        return users;
    }

    @Override
    public Collection<Integer> getGroupMembersIds(Group group) {
        return m_jdbcTemplate.queryForList(
                "select users.user_id from users join user_group on user_group.user_id=users.user_id where "
                + "user_group.group_id=" + group.getId(), Integer.class);
    }

    @Override
    public Collection<Integer> getGroupMembersByPage(int gid, int first, int pageSize) {
        Query q = getHibernateTemplate()
                .getSessionFactory()
                .getCurrentSession()
                .createSQLQuery(
                        "select users.user_id from users join user_group on user_group.user_id=users.user_id "
                        + "where user_group.group_id=:gid limit :pageSize offset :first")
                        .addScalar(USER_ID, Hibernate.INTEGER);
        q.setInteger("gid", gid);
        q.setInteger(FIRST, first);
        q.setInteger(PAGE_SIZE, pageSize);
        List<Integer> users = q.list();
        return users;
    }

    @Override
    public Collection<String> getGroupMembersNames(Group group) {
        Collection<String> userNames = getHibernateTemplate().findByNamedQueryAndNamedParam("userNamesGroupMembers",
                QUERY_PARAM_GROUP_ID, group.getId());
        return userNames;
    }

    @Override
    public int getGroupMembersCount(int groupId) {
        return m_jdbcTemplate.queryForInt(
                "select count(users.user_id) from users join user_group on user_group.user_id=users.user_id "
                + "where user_group.group_id=" + groupId);
    }

    @Override
    public int getBranchMembersCount(int branchId) {
        return m_jdbcTemplate.queryForInt(
                "select count (users.user_id) from users left outer join "
                + "user_group on users.user_id=user_group.user_id "
                + " left outer join group_storage on user_group.group_id=group_storage.group_id "
                + " where group_storage.branch_id=" + branchId + " or users.branch_id=" + branchId);
    }

    @Override
    public Collection<Integer> getBranchMembersByPage(int bid, int first, int pageSize) {
        Query q = getHibernateTemplate()
                .getSessionFactory()
                .getCurrentSession()
                .createSQLQuery(
                        "select users.user_id from users left outer join user_group on "
                        + "users.user_id=user_group.user_id "
                        + "left outer join group_storage on user_group.group_id=group_storage.group_id "
                        + "where group_storage.branch_id=:branchId or users.branch_id=:branchId "
                        + "limit :pageSize offset :first").addScalar(USER_ID, Hibernate.INTEGER);
        q.setInteger("branchId", bid);
        q.setInteger(FIRST, first);
        q.setInteger(PAGE_SIZE, pageSize);
        List<Integer> users = q.list();
        return users;
    }

    @Override
    public boolean isImIdUnique(User user) {
        ImAccount accountToSave = new ImAccount(user);
        // check ImId to save against persisted ImIds
        if (getUserProfileService().isImIdInUse(accountToSave.getImId(), user.getId())) {
            return false;
        }

        // check ImId to save against potential default ImIds
        List<ImAccount> imAccounts = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "potentialImAccountsByUserNameOrAlias", new String[] {
                    QUERY_IM_ID, QUERY_USER_ID
                }, new Object[] {
                    accountToSave.getImId(), user.getId()
                });

        for (ImAccount imAccount : imAccounts) {
            if (imAccount.getImId().equalsIgnoreCase(accountToSave.getImId())) {
                return false;
            }
        }

        return true;
    }

    @Override
    public boolean isAliasInUse(String alias) {
        Query q = getHibernateTemplate().getSessionFactory().getCurrentSession()
        .createSQLQuery(SQL_QUERY_USER_IDS_BY_NAME_OR_ALIAS).addScalar(USER_ID, Hibernate.INTEGER);
        q.setString(ALIAS, alias);
        List<Integer> userIds = q.list();
        if (SipxCollectionUtils.safeSize(userIds) > 0) {
            return true;
        }
        // check im id in user profile db
        return getUserProfileService().isImIdInUse(alias);
    }

    @Override
    public boolean isAliasInUseForOthers(String alias, String username) {
        Query q = getHibernateTemplate().getSessionFactory().getCurrentSession()
        .createSQLQuery(SQL_QUERY_USER_IDS_BY_NAME_OR_ALIAS_EXCEPT_THIS).addScalar(USER_ID, Hibernate.INTEGER);
        q.setString(ALIAS, alias);
        q.setString("username", username);
        List<Integer> userIds = q.list();
        if (SipxCollectionUtils.safeSize(userIds) > 0) {
            return true;
        }
        // check im id in user profile db
        return getUserProfileService().isAliasInUse(alias, username);
    }

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Query q = getHibernateTemplate().getSessionFactory().getCurrentSession()
        .createSQLQuery(SQL_QUERY_USER_IDS_BY_NAME_OR_ALIAS).addScalar(USER_ID, Hibernate.INTEGER);
        q.setString(ALIAS, alias);
        List<Integer> ids = q.list();
        String username = getUserProfileService().getUsernameByImId(alias);
        if (username != null) {
            User user = loadUserByUserName(username);
            Integer userId = user.getId();
            if (!ids.contains(userId)) {
                ids.add(userId);
            }
        }
        Collection bids = BeanId.createBeanIdCollection(ids, User.class);
        return bids;
    }

    @Override
    public void addToGroup(Integer groupId, Collection<Integer> ids) {
        Group group = getHibernateTemplate().load(Group.class, groupId);
        for (Integer id : ids) {
            User user = loadUser(id);
            if (!user.isGroupAvailable(group)) {
                throw new UserException("&branch.validity.error", user.getUserName(), user.getSite().getName(),
                        group.getBranch().getName());
            }
        }
        DaoUtils.addToGroup(getHibernateTemplate(), getDaoEventPublisher(), groupId, User.class, ids);
    }

    @Override
    public void removeFromGroup(Integer groupId, Collection<Integer> ids) {
        DaoUtils.removeFromGroup(getHibernateTemplate(), getDaoEventPublisher(), groupId, User.class, ids);
    }

    @Override
    public List<User> getGroupSupervisors(Group group) {
        List<User> objs = getHibernateTemplate().findByNamedQueryAndNamedParam("groupSupervisors",
                QUERY_PARAM_GROUP_ID, group.getId());
        return objs;
    }

    @Override
    public List<User> getUsersThatISupervise(User supervisor) {
        List<User> objs = getHibernateTemplate().findByNamedQueryAndNamedParam("usersThatISupervise",
                "supervisorId", supervisor.getId());
        return objs;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    /**
     * Given a collection of extensions, looks for invalid user or user without a specified
     * permission. Throw a exception if an invalid extension found.
     *
     * @param list of user aliases
     * @param permission permission to check
     * @throws ExtensionException if at least one of the aliases does not represent a valid user
     *         with permission enabled
     */
    @Override
    public void checkForValidExtensions(Collection<String> aliases, PermissionName permission) {
        Collection<String> invalidExtensions = new ArrayList<String>();
        for (String extension : aliases) {
            User user = loadUserByUserNameOrAlias(extension);
            if (user == null) {
                invalidExtensions.add(extension);
            } else if (!user.hasPermission(permission)) {
                invalidExtensions.add(extension);
            }
        }
        if (!invalidExtensions.isEmpty()) {
            throw new ExtensionException(permission, invalidExtensions);
        }
    }

    static class ExtensionException extends UserException {
        private static final String ERROR = "The following extensions do not exist or do not have {0} permission: {1}.";

        ExtensionException(PermissionName permission, Collection<String> invalidExtensions) {
            super(ERROR, permission.getName(), StringUtils.join(invalidExtensions, ", "));
        }
    }

    @Override
    public User getSpecialUser(SpecialUserType specialUserType) {
        List<SpecialUser> specialUsersOfType = getHibernateTemplate().findByNamedQueryAndNamedParam(
                SPECIAL_USER_BY_TYPE, SPECIAL_USER_TYPE, specialUserType.name());
        SpecialUser specialUser = DataAccessUtils.singleResult(specialUsersOfType);
        if (specialUser == null) {
            return null;
        }

        User newUser = newUser();
        newUser.setUserName(specialUser.getUserName());
        newUser.setSipPassword(specialUser.getSipPassword());
        return newUser;
    }

    @Override
    public SpecialUser getSpecialUserAsSpecialUser(SpecialUserType specialUserType) {
        List<SpecialUser> specialUsersOfType = getHibernateTemplate().findByNamedQueryAndNamedParam(
                SPECIAL_USER_BY_TYPE, SPECIAL_USER_TYPE, specialUserType.name());
        SpecialUser specialUser = DataAccessUtils.singleResult(specialUsersOfType);
        if (specialUser == null) {
            return null;
        }
        return specialUser;
    }

    @Override
    public void initializeSpecialUsers() {
        for (SpecialUserType type : SpecialUserType.values()) {
            User specialUser = getSpecialUser(type);
            if (specialUser == null) {
                SpecialUser newSpecialUser = new SpecialUser(type);
                getHibernateTemplate().saveOrUpdate(newSpecialUser);
                getDaoEventPublisher().publishSave(newSpecialUser);
            }
        }
    }

    /*
     * Here take account only of the special users. In ReplicationManagerImpl.generateAll all
     * usersare replicated separately
     *
     * @see org.sipfoundry.sipxconfig.common.ReplicableProvider#getReplicables()
     */
    @Override
    public List<Replicable> getReplicables() {
        List<Replicable> replicables = new ArrayList<Replicable>();
        for (SpecialUserType specialUserType : SpecialUserType.values()) {
            SpecialUser user = getSpecialUserAsSpecialUser(specialUserType);
            replicables.add(user);
        }
        replicables.addAll(getGroups());
        return replicables;
    }

    public void setConfigJdbcTemplate(JdbcTemplate jdbcTemplate) {
        m_jdbcTemplate = jdbcTemplate;
    }

    @Override
    public boolean setup(SetupManager manager) {
        if (!m_setup) {
            // this checks special users on every start-up as there seems to be no harm.
            initializeSpecialUsers();
            m_setup = true;
        }
        return true;
    }
}
