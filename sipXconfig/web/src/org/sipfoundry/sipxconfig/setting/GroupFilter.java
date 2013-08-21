/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.Messages;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroup;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;

import static org.sipfoundry.commons.userdb.profile.UserProfileService.DISABLED;
import static org.sipfoundry.commons.userdb.profile.UserProfileService.ENABLED;

public abstract class GroupFilter extends BaseComponent {
    public static final String SEARCH_SELECTED = "label.search";

    public static final String ALL_SELECTED = "label.all";

    public static final String UNASSIGNED_SELECTED = "label.unassigned";

    public static final String ENABLED_SELECTED = "label.enabled";

    public static final String DISABLED_SELECTED = "label.disabled";

    public abstract boolean getSearchMode();

    public abstract void setSearchMode(boolean searchMode);

    public abstract Integer getSelectedGroupId();

    public abstract void setSelectedGroupId(Integer groupId);

    public abstract Object getGroupId();

    public abstract void setGroupId(Object groupId);

    public abstract String getQueryText();

    public abstract void setQueryText(String queryText);

    @Parameter(required = false, defaultValue = "ognl:new java.util.ArrayList()")
    public abstract Collection getGroups();

    public abstract TapestryContext getTapestry();

    public abstract boolean getUnassignedOptionAvailable();

    public abstract void setUnassignedOptionAvailable(boolean unassignedOptionAvailable);

    public abstract boolean getEnabledOptionAvailable();

    public abstract void setEnabledOptionAvailable(boolean enabledOptionAvailable);

    public abstract boolean getEnabledMode();

    public abstract void setEnabledMode(boolean enabledMode);

    public abstract boolean getDisabledOptionAvailable();

    public abstract void setDisabledOptionAvailable(boolean disabledOptionAvailable);

    public abstract boolean getDisabledMode();

    public abstract void setDisabledMode(boolean disabledMode);

    public abstract boolean getUnassignedMode();

    public abstract void setUnassignedMode(boolean unassignedMode);

    public IPropertySelectionModel getSelectionModel() {
        Collection actions = new ArrayList();

        actions.add(new LabelOptionAdapter(getMessages(), ALL_SELECTED));
        if (getEnabledOptionAvailable()) {
            actions.add(new LabelOptionAdapter(getMessages(), ENABLED_SELECTED));
        }
        if (getDisabledOptionAvailable()) {
            actions.add(new LabelOptionAdapter(getMessages(), DISABLED_SELECTED));
        }
        if (getUnassignedOptionAvailable()) {
            actions.add(new LabelOptionAdapter(getMessages(), UNASSIGNED_SELECTED));
        }
        actions.add(new LabelOptionAdapter(getMessages(), SEARCH_SELECTED));

        Collection groups = getGroups();
        if (!groups.isEmpty()) {
            actions.add(new OptGroup(getMessages().getMessage("label.groups")));
        }
        for (Iterator i = groups.iterator(); i.hasNext();) {
            Group g = (Group) i.next();
            actions.add(new GroupOptionAdapter(g));
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);
        return getTapestry().addExtraOption(model, getMessages(), "label.filter");
    }

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        if (!cycle.isRewinding()) {
            Integer groupId = getSelectedGroupId();
            if (getUnassignedOptionAvailable() && getUnassignedMode()) {
                setGroupId(UNASSIGNED_SELECTED);
            } else if (getEnabledOptionAvailable() && getEnabledMode()) {
                setGroupId(ENABLED_SELECTED);
            } else if (getDisabledOptionAvailable() && getDisabledMode()) {
                setGroupId(DISABLED_SELECTED);
            } else if (getSearchMode()) {
                if (StringUtils.equals(getQueryText(), ENABLED)) {
                    setGroupId(ENABLED_SELECTED);
                } else if (StringUtils.equals(getQueryText(), DISABLED)) {
                    setGroupId(DISABLED_SELECTED);
                } else {
                    setGroupId(SEARCH_SELECTED);
                }
            } else {
                setGroupId(groupId);
            }
        }
        super.renderComponent(writer, cycle);
        // when rewinding
        if (cycle.isRewinding() && getGroupId() != null) {
            OptionAdapter option = (OptionAdapter) getGroupId();
            Object groupId = option.getValue(null, 0);
            final boolean search = SEARCH_SELECTED.equals(groupId);
            final boolean all = ALL_SELECTED.equals(groupId);
            final boolean enabled = ENABLED_SELECTED.equals(groupId);
            final boolean disabled = DISABLED_SELECTED.equals(groupId);
            final boolean unassigned = UNASSIGNED_SELECTED.equals(groupId);
            setSearchMode(search);
            if (getUnassignedOptionAvailable()) {
                setUnassignedMode(unassigned);
            }
            if (getEnabledOptionAvailable() && enabled) {
                setEnabledMode(enabled);
                setSearchMode(enabled);
                setQueryText(ENABLED);
            }
            if (getDisabledOptionAvailable() && disabled) {
                setDisabledMode(disabled);
                setSearchMode(disabled);
                setQueryText(DISABLED);
            }
            if (search
                    && (StringUtils.equals(getQueryText(), ENABLED) || StringUtils.equals(
                            getQueryText(), DISABLED))) {
                setQueryText(null);
            }
            if (search || all || unassigned || enabled || disabled) {
                setSelectedGroupId(null);
            } else {
                setSelectedGroupId((Integer) groupId);
            }
        }
    }

    public boolean renderSearchQueryAndButton(String queryText) {
        if (!StringUtils.equals(queryText, ENABLED)
                && !StringUtils.equals(queryText, DISABLED)) {
            return true;
        }
        return false;
    }

    /**
     * LabelOptionAdapter
     */
    private static class LabelOptionAdapter implements OptionAdapter {
        private final String m_label;
        private final Messages m_messages;

        public LabelOptionAdapter(Messages messages, String label) {
            m_messages = messages;
            m_label = label;
        }

        public Object getValue(Object option_, int index_) {
            return m_label;
        }

        public String getLabel(Object option_, int index_) {
            return m_messages.getMessage(m_label);
        }

        public String squeezeOption(Object option_, int index_) {
            return m_label;
        }
    }

    private static class GroupOptionAdapter implements OptionAdapter {
        private final Group m_group;

        public GroupOptionAdapter(Group group) {
            m_group = group;
        }

        public Object getValue(Object option_, int index_) {
            return m_group.getId();
        }

        public String getLabel(Object option_, int index_) {
            return m_group.getName();
        }

        public String squeezeOption(Object option_, int index_) {
            return m_group.getId().toString();
        }
    }

    /**
     * TODO: this is alternative implementation - a single option adapter
     */
    public class FilterSelectorAdapter implements OptionAdapter {
        public Object getValue(Object option, int index_) {
            if (option instanceof Group) {
                Group g = (Group) option;
                return g.getId();
            }
            return option;
        }

        public String getLabel(Object option, int index_) {
            if (option instanceof Group) {
                Group g = (Group) option;
                return g.getName();
            }
            if (option instanceof String) {
                String label = (String) option;
                return getMessages().getMessage(label);
            }
            return option.toString();
        }

        public String squeezeOption(Object option, int index_) {
            if (option instanceof Group) {
                Group g = (Group) option;
                return g.getId().toString();
            }
            return option.toString();
        }
    }
}
