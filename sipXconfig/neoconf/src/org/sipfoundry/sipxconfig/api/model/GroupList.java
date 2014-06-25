/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.api.model;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.sipfoundry.sipxconfig.setting.Group;

@XmlRootElement(name = "Groups")
public class GroupList {
    private List<GroupBean> m_groups;

    public void setGroups(List<GroupBean> groups) {
        m_groups = groups;
    }

    @XmlElement(name = "Group")
    public List<GroupBean> getGroups() {
        if (m_groups == null) {
            m_groups = new ArrayList<GroupBean>();
        }
        return m_groups;
    }

    public static GroupList convertGroupList(List<Group> groups, Map count) {
        GroupList list = new GroupList();
        list.setGroups(GroupBean.buildGroupList(groups, count));
        return list;
    }
}
