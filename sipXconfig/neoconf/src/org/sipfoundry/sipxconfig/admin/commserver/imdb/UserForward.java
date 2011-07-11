/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.jdbc.core.RowCallbackHandler;

public class UserForward extends DataSetGenerator {

    private static final String QUERY = "SELECT user_id, user_name, v.value as cfwd, "
            + "(SELECT count(*) from user_group where user_id = u.user_id) as groups FROM users u "
            + "left join setting_value v on u.value_storage_id = v.value_storage_id "
            + "and v.path='callfwd/timer' WHERE u.user_type='C' ORDER BY u.user_id;";

    @Override
    protected DataSet getType() {
        return DataSet.USER_FORWARD;
    }

    @Override
    protected void addItems(final List<Map<String, String>> items) {
        final String domainName = getSipDomain();
        final User user = getCoreContext().newUser();
        List<Group> groups = getCoreContext().getGroups();
        final Map<Integer, Group> groupsMap = new HashMap<Integer, Group>();
        for (Group group : groups) {
            groupsMap.put(group.getId(), group);
        }

        getJdbcTemplate().query(QUERY, new RowCallbackHandler() {

            @Override
            public void processRow(ResultSet rs) throws SQLException {
                String userId = rs.getString("user_id");
                String userName = rs.getString("user_name");
                int groupsCount = rs.getInt("groups");
                user.setGroups(new TreeSet<Group>());
                if (groupsCount > 0) {
                    // add groups to this user model
                    getJdbcTemplate().query("SELECT u.group_id from user_group u inner join group_storage s "
                            + "on u.group_id = s.group_id WHERE user_id=" + userId
                            + " AND s.resource='user';", new RowCallbackHandler() {

                                @Override
                                public void processRow(ResultSet rs) throws SQLException {
                                    user.addGroup(groupsMap.get(rs.getInt("group_id")));
                                }
                            });
                }
                String defaultCfwd = user.getSettings().getSetting(AbstractUser.CALLFWD_TIMER).getDefaultValue();
                String cfwdTime = StringUtils.defaultIfEmpty(rs.getString("cfwd"), defaultCfwd);
                addUser(items, userName, domainName, cfwdTime);
            }
        });
    }

    protected void addUser(List<Map<String, String>> items, String userName, String domainName, String cfwdtime) {
        Map<String, String> item = addItem(items);
        String identity = userName + "@" + domainName;
        item.put("identity", identity);
        item.put("cfwdtime", cfwdtime);
    }
}
