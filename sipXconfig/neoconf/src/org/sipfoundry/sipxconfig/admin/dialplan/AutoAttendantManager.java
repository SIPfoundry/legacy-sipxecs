/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.setting.Group;

public interface AutoAttendantManager extends AliasOwner {
    String ATTENDANT_GROUP_ID = "auto_attendant";

    void clear();

    void storeAutoAttendant(AutoAttendant attendant);

    void deleteAutoAttendant(AutoAttendant attendant);

    AutoAttendant getAutoAttendant(Integer id);

    List<AutoAttendant> getAutoAttendants();

    void deleteAutoAttendantsByIds(Collection<Integer> attendantsIds);

    void specialAutoAttendantMode(boolean enabled, AutoAttendant attendant);

    Group getDefaultAutoAttendantGroup();

    AutoAttendant newAutoAttendantWithDefaultGroup();

    AutoAttendant getOperator();

    AutoAttendant createOperator(String attendantId);
}
