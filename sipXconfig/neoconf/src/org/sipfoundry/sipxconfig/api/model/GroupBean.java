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

import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;

import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.Group;

@XmlRootElement(name = "group")
@XmlType(propOrder = {
        "id", "name", "description", "weight", "count"
        })
@JsonPropertyOrder({
        "id", "name", "description", "weight", "count"
    })
public class GroupBean {
    private int m_id;
    private String m_name;
    private String m_description;
    private Integer m_weight;
    private Long m_count;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getName() {
        return m_name;
    }

    public void setWeight(int weight) {
        m_weight = weight;
    }

    public int getWeight() {
        return m_weight;
    }

    public Long getCount() {
        return m_count;
    }

    public void setCount(Long count) {
        m_count = count;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public static List<GroupBean> buildGroupList(List<Group> phoneGroups, Map count) {
        List<GroupBean> groups = new LinkedList<GroupBean>();
        for (Group group : phoneGroups) {
            GroupBean groupBean = new GroupBean();
            convertGroup(group, groupBean);
            groupBean.setCount(count != null ? (Long) count.get(group.getId()) : null);
            groups.add(groupBean);
        }
        if (groups.size() > 0) {
            return groups;
        }
        return null;
    }

    public static void convertGroup(Group group, GroupBean groupBean) {
        Integer id = group.getId();
        groupBean.setId(id);
        groupBean.setName(group.getName());
        groupBean.setWeight(group.getWeight());
        groupBean.setDescription(group.getDescription());
    }

    private  static void convertToGroup(GroupBean groupBean, Group group, String resource) {
        group.setName(groupBean.getName());
        group.setWeight(groupBean.getWeight());
        group.setDescription(groupBean.getDescription());
        group.setResource(resource);
    }

    public static void convertToPhoneGroup(GroupBean groupBean, Group group) {
        convertToGroup(groupBean, group, Phone.GROUP_RESOURCE_ID);
    }
}
