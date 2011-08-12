/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
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
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.jdbc.core.RowCallbackHandler;

public class UserLocation extends DataSetGenerator {

    private static final String QUERY = "SELECT u.user_id, u.user_name, b.name as branch_name, "
            + "(SELECT count(*) from user_group where user_id = u.user_id) as groups FROM users u "
            + "left join branch b on u.branch_id = b.branch_id " + "WHERE u.user_type='C' ORDER BY u.user_id;";

    @Override
    protected DataSet getType() {
        return DataSet.USER_LOCATION;
    }

    @Override
    protected void addItems(final List<Map<String, String>> items) {
        final String domainName = getSipDomain();
        final User userModel = getCoreContext().newUser();

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
                String branchName = rs.getString("branch_name");
                int groupsCount = rs.getInt("groups");
                userModel.setGroups(new TreeSet<Group>());
                if (groupsCount > 0) {
                    // add groups to this user model
                    getJdbcTemplate().query("SELECT u.group_id from user_group u inner join group_storage s "
                            + "on u.group_id = s.group_id WHERE user_id=" + userId
                            + " AND s.resource='user';", new RowCallbackHandler() {

                                @Override
                                public void processRow(ResultSet rs) throws SQLException {
                                    userModel.addGroup(groupsMap.get(rs.getInt("group_id")));
                                }
                            });
                }
                if (StringUtils.isEmpty(branchName)) {
                    Branch branch = userModel.getSite();
                    if (branch != null) {
                        branchName = branch.getName();
                    }
                }
                if (!StringUtils.isEmpty(branchName)) {
                    addUser(items, userName, domainName, branchName);
                }
            }
        });
    }

    protected void addUser(List<Map<String, String>> items, String userName, String domainName, String branchName) {
        String url = SipUri.format(userName, domainName, false);
        addUserLocationItem(items, url, branchName);
    }

    private void addUserLocationItem(List<Map<String, String>> items, String url, String siteName) {
        Map<String, String> item = addItem(items);
        item.put("identity", url);
        item.put("location", siteName);
    }
}
