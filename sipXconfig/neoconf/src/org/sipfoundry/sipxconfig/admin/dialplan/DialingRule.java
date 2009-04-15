/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.forwarding.Schedule;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.DataCollectionItem;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionManager;

/**
 * DialingRule At some point it will be replaced by the IDialingRule interface or made abstract.
 */
public abstract class DialingRule extends BeanWithId implements DataCollectionItem, NamedObject, IDialingRule {

    public static final String VALID_TIME_PARAM = "sipx-ValidTime=%s";
    public static final String GATEWAY_EXPIRES_PATTERN = "expires=%s";
    public static final String GATEWAY_EXPIRES_VALUE = "60";

    private boolean m_enabled;
    private String m_name;
    private String m_description;
    private int m_position;
    private List<Gateway> m_gateways = new ArrayList<Gateway>();
    private transient PermissionManager m_permissionManager;
    private Schedule m_schedule;

    public abstract String[] getPatterns();

    public abstract Transform[] getTransforms();

    public abstract DialingRuleType getType();

    @Override
    protected Object clone() throws CloneNotSupportedException {
        DialingRule clone = (DialingRule) super.clone();
        clone.m_gateways = new ArrayList<Gateway>(m_gateways);
        return clone;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getRuleType() {
        return null;
    }

    public List<Gateway> getGateways() {
        return m_gateways;
    }

    public void setGateways(List<Gateway> gateways) {
        m_gateways = gateways;
    }

    public Schedule getSchedule() {
        return m_schedule;
    }

    public void setSchedule(Schedule schedule) {
        m_schedule = schedule;
    }

    /**
     * Check if permission should be applied to the rule target.
     *
     * Permissions in a dialing rule can apply to target or to source. In most cases permissions
     * apply to a rule source (i.e. caller).
     *
     * There are a few exceptions: in voicemail rules permissions are applied to target.
     */
    public boolean isTargetPermission() {
        return false;
    }

    public final List<Permission> getPermissions() {
        List<String> permissionNames = getPermissionNames();
        List<Permission> permissions = new ArrayList<Permission>(permissionNames.size());
        for (String name : permissionNames) {
            Permission permission = getPermission(name);
            if (permission != null) {
                permissions.add(permission);
            }
        }
        return permissions;
    }

    public List<String> getPermissionNames() {
        return Collections.emptyList();
    }

    /**
     * @return list of Gateway objects representing source hosts
     */
    public List<Gateway> getHosts() {
        return null;
    }

    public boolean addGateway(Gateway gateway) {
        boolean existed = !m_gateways.remove(gateway);
        m_gateways.add(gateway);
        return existed;
    }

    public void removeGateways(Collection<Integer> selectedGateways) {
        for (Iterator<Integer> i = selectedGateways.iterator(); i.hasNext();) {
            Integer id = i.next();
            m_gateways.remove(new BeanWithId(id));
        }
    }

    /**
     * Called to give a dialing rules a chance to append itself to the list of rules used for
     * generating XML
     *
     * Default implementation appends the rule if it is enabled. Rule can append some other rules.
     *
     * @param rules
     */
    public void appendToGenerationRules(List<DialingRule> rules) {
        if (isEnabled()) {
            rules.add(this);
        }
    }

    /**
     * Returns the list of gateways that can be added to this rule.
     *
     * @param allGateways pool of all possible gateways
     * @return list of gateways that still can be assigned to this rule
     */
    public Collection<Gateway> getAvailableGateways(List<Gateway> allGateways) {
        Set<Gateway> gateways = new HashSet<Gateway>(allGateways);
        Collection<Gateway> ruleGateways = getGateways();
        gateways.removeAll(ruleGateways);
        return gateways;
    }

    public void moveGateways(Collection<Integer> ids, int step) {
        DataCollectionUtil.moveByPrimaryKey(m_gateways, ids.toArray(), step, false);
    }

    public int getPosition() {
        return m_position;
    }

    public void setPosition(int position) {
        m_position = position;
    }

    @Override
    public Object getPrimaryKey() {
        return getId();
    }

    /**
     * Attempts to apply standard transformations to patterns. It is used when generating
     * authorization rules For example 9xxxx -> 7{digits} results in 97xxxx
     *
     * Default implementation inserts gateway specific prefixes.
     */
    public String[] getTransformedPatterns(Gateway gateway) {
        String[] patterns = getPatterns();
        String[] transformed = new String[patterns.length];
        for (int i = 0; i < patterns.length; i++) {
            if (gateway != null) {
                transformed[i] = gateway.getCallPattern(patterns[i]);
            } else {
                transformed[i] = patterns[i];
            }
        }
        return transformed;
    }

    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }

    protected Permission getPermission(String name) {
        if (name == null) {
            return null;
        }
        if (m_permissionManager == null) {
            throw new IllegalStateException("Permission manager not configured.");
        }
        return m_permissionManager.getPermissionByName(Permission.Type.CALL, name);
    }

    public String[] getHostPatterns() {
        return ArrayUtils.EMPTY_STRING_ARRAY;
    }

    public Map<String, List<Transform>> getSiteTransforms() {
        Transform[] transforms = getTransforms();
        if (transforms == null) {
            return null;
        }
        Map<String, List<Transform>> siteTransforms = new HashMap<String, List<Transform>>(1);
        siteTransforms.put(StringUtils.EMPTY, Arrays.asList(transforms));
        return siteTransforms;
    }
}
