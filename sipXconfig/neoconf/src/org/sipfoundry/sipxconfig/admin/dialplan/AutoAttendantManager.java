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

import java.io.File;
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

    AutoAttendant getAutoAttendantBySystemName(String systemName);

    List<AutoAttendant> getAutoAttendants();

    void deleteAutoAttendantsByIds(Collection<Integer> attendantsIds);

    Group getDefaultAutoAttendantGroup();

    AutoAttendant newAutoAttendantWithDefaultGroup();

    AutoAttendant getOperator();

    AutoAttendant createOperator(String attendantId);

    /**
     * Get new default prompts autoattendant.wav (operator Autoattendant) and afterhours.wav
     * (afterhour Autoattendant) from given directory. Used when a different localization language
     * is set
     *
     * @param sourceDir - path directory where new prompts are saved
     */
    void updatePrompts(File sourceDir);

    void selectSpecial(AutoAttendant attendant);

    void deselectSpecial(AutoAttendant attendant);

    void setSpecialMode(boolean enabled);

    boolean getSpecialMode();

    AutoAttendant getSelectedSpecialAttendant();
}
