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

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;

import com.mongodb.BasicDBObject;

public class OpenAcdRecipeAction extends OpenAcdQueueWithSkills implements Serializable {
    private static final String AT_SIGN = "@";

    public static enum ACTION {
        ADD_SKILLS {
            public String toString() {
                return "add_skills";
            }
        },
        REMOVE_SKILLS {
            public String toString() {
                return "remove_skills";
            }
        },
        SET_PRIORITY {
            public String toString() {
                return "set_priority";
            }
        },
        PRIORITIZE {
            public String toString() {
                return "prioritize";
            }
        },
        DEPRIORITIZE {
            public String toString() {
                return "deprioritize";
            }
        },
        SEND_TO_VOICEMAIL {
            public String toString() {
                return "voicemail";
            }
        },
        MEDIA_ANNOUCE {
            public String toString() {
                return "announce";
            }
        },
        TRANSFER_OUTBAND {
            public String toString() {
                return "transfer_outband";
            }
        }
    }

    private String m_action = ACTION.PRIORITIZE.toString();

    private String m_actionValue;

    public String getAction() {
        return m_action;
    }

    public void setAction(String action) {
        m_action = action;
    }

    public String getActionValue() {
        return m_actionValue;
    }

    public void setActionValue(String actionValue) {
        m_actionValue = actionValue;
    }

    public int hashCode() {
        return new HashCodeBuilder().append(m_action).append(m_actionValue).toHashCode();
    }

    public boolean equals(Object other) {
        if (!(other instanceof OpenAcdRecipeStep)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        OpenAcdRecipeAction bean = (OpenAcdRecipeAction) other;
        return new EqualsBuilder().append(m_action, bean.getAction()).append(m_actionValue, bean.getActionValue())
                .isEquals();
    }

    public BasicDBObject getMongoObject(String domain) {
        BasicDBObject action = new BasicDBObject();
        action.put("action", m_action);
        String actionValue = "";
        if (m_action.equals(ACTION.ADD_SKILLS.toString()) || m_action.equals(ACTION.REMOVE_SKILLS.toString())) {
            action.put(OpenAcdContext.ACTION_VALUE, getSkillsAtoms());
        } else if (m_action.equals(ACTION.SET_PRIORITY.toString())
                || m_action.equals(ACTION.MEDIA_ANNOUCE.toString())) {
            actionValue = getActionValue();
            action.put(OpenAcdContext.ACTION_VALUE, actionValue);
        } else if (m_action.equals(ACTION.TRANSFER_OUTBAND.toString())) {
            actionValue = getActionValue();
            if (StringUtils.contains(actionValue, AT_SIGN)) {
                // external sip uri
                action.put(OpenAcdContext.ACTION_VALUE, actionValue);
            } else {
                // internal extension
                StringBuilder builder = new StringBuilder();
                builder.append(actionValue).append(AT_SIGN).append(domain);
                action.put(OpenAcdContext.ACTION_VALUE, builder.toString());
            }
        }

        return action;
    }
}
