/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.openacd;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.common.BeanWithId;

public class OpenAcdRecipeStep extends BeanWithId implements Serializable {

    public static enum FREQUENCY {
        RUN_ONCE {
            public String toString() {
                return "run_once";
            }
        },
        RUN_MANY {
            public String toString() {
                return "run_many";
            }
        }
    }

    private String m_name;

    private String m_description;

    private OpenAcdRecipeAction m_action = new OpenAcdRecipeAction();

    private List<OpenAcdRecipeCondition> m_conditions = new ArrayList<OpenAcdRecipeCondition>();

    private String m_frequency;

    public OpenAcdRecipeStep() {
        OpenAcdRecipeCondition condition = new OpenAcdRecipeCondition();
        m_conditions.add(condition);
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public OpenAcdRecipeAction getAction() {
        return m_action;
    }

    public void setAction(OpenAcdRecipeAction action) {
        m_action = action;
    }

    public List<OpenAcdRecipeCondition> getConditions() {
        return m_conditions;
    }

    public void setConditions(List<OpenAcdRecipeCondition> conditions) {
        m_conditions = conditions;
    }

    public void addCondition(OpenAcdRecipeCondition condition) {
        m_conditions.add(condition);
    }

    public void removeCondition(OpenAcdRecipeCondition condition) {
        m_conditions.remove(condition);
    }

    public String getFrequency() {
        return m_frequency;
    }

    public void setFrequency(String frequency) {
        m_frequency = frequency;
    }

    public int hashCode() {
        return new HashCodeBuilder().append(m_action).append(m_frequency).append(m_conditions).toHashCode();
    }

    public boolean equals(Object other) {
        if (!(other instanceof OpenAcdRecipeStep)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        OpenAcdRecipeStep bean = (OpenAcdRecipeStep) other;
        return new EqualsBuilder().append(m_name, bean.getName()).append(m_frequency, bean.getFrequency())
                .append(m_action.getId(), bean.getAction().getId()).isEquals();
    }

}
