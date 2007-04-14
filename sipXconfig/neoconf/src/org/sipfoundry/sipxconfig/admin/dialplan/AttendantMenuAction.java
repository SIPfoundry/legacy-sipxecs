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

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.common.EnumUserType;

/**
 * All possible, supported actions for creation auto attendants. 
 */
public final class AttendantMenuAction extends Enum {
    
    public static final AttendantMenuAction OPERATOR = new AttendantMenuAction("operator");
    
    public static final AttendantMenuAction DIAL_BY_NAME = new AttendantMenuAction("dial_by_name");

    public static final AttendantMenuAction REPEAT_PROMPT = new AttendantMenuAction("repeat_prompt");

    public static final AttendantMenuAction VOICEMAIL_LOGIN = new AttendantMenuAction("voicemail_access");

    public static final AttendantMenuAction DISCONNECT = new AttendantMenuAction("disconnect");

    public static final AttendantMenuAction AUTO_ATTENDANT = new AttendantMenuAction("transfer_to_another_aa_menu");
    
    public static final AttendantMenuAction GOTO_EXTENSION  = new AttendantMenuAction("transfer_to_extension");
    
    public static final AttendantMenuAction VOICEMAIL_DEPOSIT  = new AttendantMenuAction("voicemail_deposit");
    
    
    public static final AttendantMenuAction[] ACTIONS = 
            (AttendantMenuAction[]) getEnumList(AttendantMenuAction.class).toArray(new AttendantMenuAction[0]);

    private AttendantMenuAction(String actionId) {
        super(actionId);
    }
    
    public boolean isExtensionParameter() {
        return (this == VOICEMAIL_DEPOSIT || this == GOTO_EXTENSION);
    }
    
    public boolean isAttendantParameter() {
        return (this == AUTO_ATTENDANT);
    }
    
    public String vxmlParameter(String parameter) {
        if (this == OPERATOR) {
            return "operatoraddr";
        }
        
        return StringUtils.isBlank(parameter) ? "none" : parameter;        
    }
    
    /**
     * Used for Hibernate type translation
     */
    public static class UserType extends EnumUserType {
        public UserType() {
            super(AttendantMenuAction.class);
        }
    }
}
