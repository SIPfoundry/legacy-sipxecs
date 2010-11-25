/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.freeswitch;

import java.util.LinkedHashSet;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.common.BeanWithId;

public class FreeswitchCondition extends BeanWithId {

    private String m_field;
    private String m_expression;
    private Set<FreeswitchAction> m_actions = new LinkedHashSet<FreeswitchAction>();

    public String getField() {
        return m_field;
    }

    public void setField(String field) {
        m_field = field;
    }

    public String getExpression() {
        return m_expression;
    }

    public void setExpression(String expression) {
        m_expression = expression;
    }

    public Set<FreeswitchAction> getActions() {
        return m_actions;
    }

    public void setActions(Set<FreeswitchAction> actions) {
        m_actions = actions;
    }

    public void addAction(FreeswitchAction action) {
        m_actions.add(action);
    }

    public String getLineNumber() {
        return StringUtils.removeEnd(StringUtils.removeStart(m_expression, "^"), "$");
    }

    @Override
    public int hashCode() {
        return new HashCodeBuilder().append(m_field).append(m_expression).toHashCode();
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof FreeswitchCondition)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        FreeswitchCondition bean = (FreeswitchCondition) other;
        return new EqualsBuilder().append(m_field, bean.m_field).append(m_expression, bean.m_expression).isEquals();
    }
}
