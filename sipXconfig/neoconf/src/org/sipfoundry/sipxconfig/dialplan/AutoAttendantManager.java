/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.dialplan;

import java.io.File;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.setting.Group;

public interface AutoAttendantManager extends AliasOwner {
    final GlobalFeature FEATURE = new GlobalFeature("autoAttendant");
    String ATTENDANT_GROUP_ID = "auto_attendant";

    void clear();

    void storeAutoAttendant(AutoAttendant attendant);

    void deleteAutoAttendant(AutoAttendant attendant);

    AutoAttendant getAutoAttendant(Integer id);

    AutoAttendant getAutoAttendantBySystemName(String systemName);

    List<AutoAttendant> getAutoAttendants();

    void deleteAutoAttendantsByIds(Collection<Integer> attendantsIds);

    void dupeAutoAttendants(Collection<Integer> attendantsIds);

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

    void deselectSpecial(AutoAttendant attendant);

    void setAttendantSpecialMode(boolean enabled, AutoAttendant attendant);

    boolean getSpecialMode();

    AutoAttendant getSelectedSpecialAttendant();
}
