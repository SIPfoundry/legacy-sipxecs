/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.apache.commons.collections.Transformer;
import org.apache.commons.collections.iterators.TransformIterator;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.NamedObject;

/**
 * Common code for line, phone, and user information.
 */
public abstract class BeanWithGroups extends BeanWithSettings {
    public static final String GROUPS_PROP = "groups";

    private Set<Group> m_groups;

    @Override
    protected void initializeSettingModel() {
        setSettingModel2(new BeanWithGroupsModel(this));
    }

    public synchronized Set<Group> getGroups() {
        // lazy to avoid NPE in unit tests that create mock objects for subclasses
        if (m_groups == null) {
            setGroups(new TreeSet<Group>());
        }
        return m_groups;
    }

    public void setGroups(Set<Group> settingSets) {
        m_groups = settingSets;

        BeanWithGroupsModel model = (BeanWithGroupsModel) getSettingModel2();
        // passed collection is not copied
        model.setGroups(m_groups);
    }

    public List<Group> getGroupsAsList() {
        return new ArrayList<Group>(getGroups());
    }

    public void setGroupsAsList(List<Group> groups) {
        getGroups().clear();
        getGroups().addAll(groups);
    }

    public Group getFirstGroupDefaultIsSetFor(Setting setting) {
        for (Group g : getGroups()) {
            if (g.getSettingValue(setting) != null) {
                return g;
            }
        }

        return null;
    }

    /**
     * Finds the first group for which branch is defined and returns that branch.
     *
     * Should be used when determining the effective branch for phone and user.
     *
     * @return effective branch for this bean
     */
    public Branch getInheritedBranch() {
        for (Group group : getGroups()) {
            Branch branch = group.getBranch();
            if (branch != null) {
                return branch;
            }
        }
        return null;
    }

    /**
     * @return string representation of groups as space separated group names
     */
    public String getGroupsNames() {
        return getGroupsAsString(getGroups());
    }

    /**
     * Determines if a group can be added in the list of groups. If this group has a
     * different branch than an existing group it cannot be added
     * @return
     */
    public boolean isGroupAvailable(Group group) {
        for (Group tmpGroup : getGroups()) {
            if (tmpGroup.getBranch() != null && group.getBranch() != null
                    && !StringUtils.equals(tmpGroup.getBranch().getName(), group.getBranch().getName())) {
                return false;
            }
        }
        return true;
    }

    public void addGroup(Group tag) {
        getGroups().add(tag);
    }

    public void removeGroup(Group tag) {
        getGroups().remove(tag);
    }

    /**
     * @param groups collection of groups - emtpy and null collections OK
     * @return string representation of groups as space separated group names
     */
    public static String getGroupsAsString(Collection< ? extends NamedObject> groups) {
        if (groups == null) {
            return StringUtils.EMPTY;
        }
        TransformIterator namesIterator = new TransformIterator(groups.iterator(), new NamedObject.ToName());
        return StringUtils.join(namesIterator, " ");
    }

    public static class AddTag implements Transformer {
        private final Group m_tag;

        public AddTag(Group tag) {
            m_tag = tag;
        }

        public Object transform(Object input) {
            BeanWithGroups bean = (BeanWithGroups) input;
            bean.addGroup(m_tag);
            return bean;
        }
    }

    public static class RemoveTag implements Transformer {
        private final Group m_tag;

        public RemoveTag(Group tag) {
            m_tag = tag;
        }

        public Object transform(Object input) {
            BeanWithGroups bean = (BeanWithGroups) input;
            bean.removeGroup(m_tag);
            return bean;
        }
    }
}
