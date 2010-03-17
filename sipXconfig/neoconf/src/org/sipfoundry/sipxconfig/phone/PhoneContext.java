/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.io.Serializable;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.intercom.Intercom;
import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public interface PhoneContext extends DataObjectSource {

    String GROUP_RESOURCE_ID = "phone";
    String CONTEXT_BEAN_NAME = "phoneContext";

    /**
     * Commits the transaction and performs a batch of SQL commands to database. Call this as high
     * in the application stack as possible for better performance and data integrity.
     *
     * You need to call this before you attempt to delete an object that was created before last
     * call to flush. Not unreasonable, most times you don't delete and object before it's
     * created, but happens a lot in unittests.
     */
    void flush();

    int getPhonesCount();

    int getPhonesInGroupCount(Integer groupId);

    List<Phone> loadPhonesByPage(Integer groupId, int page, int pageSize, String[] orderBy,
            boolean orderAscending);

    List<Phone> loadPhones();

    List<Integer> getAllPhoneIds();

    void storeLine(Line line);

    void deleteLine(Line line);

    Line loadLine(Integer id);

    Phone newPhone(PhoneModel model);

    Phone loadPhone(Integer id);

    Integer getPhoneIdBySerialNumber(String serialNumber);

    Object load(Class c, Serializable id);

    void storePhone(Phone phone);

    void deletePhone(Phone phone);

    List<Group> getGroups();

    /**
     * Retrieves phone group by name.
     *
     * @param phoneGroupName name of the group
     * @param createIfNotFound if true a new group with this name will be created, if false null
     *        is returned if group with a phoneGroupName is not found
     * @return phone group or null if group not found and createIfNotFound is false
     */
    Group getGroupByName(String phoneGroupName, boolean createIfNotFound);

    /** unittesting only */
    void clear();

    String getSystemDirectory();

    DeviceDefaults getPhoneDefaults();

    Collection<Phone> getPhonesByGroupId(Integer groupId);

    Collection<Phone> getPhonesByUserId(Integer userId);

    void addToGroup(Integer groupId, Collection<Integer> ids);

    void removeFromGroup(Integer groupId, Collection<Integer> ids);

    void addUsersToPhone(Integer phoneId, Collection<Integer> ids);

    Collection<Integer> getPhoneIdsByUserGroupId(int groupId);

    /**
     * Return the intercom associated with a phone, through the groups the phone belongs to, or
     * null if there is no intercom for the phone. There should be at most one intercom for any
     * phone. If there is more than one, then return the first intercom found.
     */
    Intercom getIntercomForPhone(Phone phone);

    /**
     * Phonebook/Directory/Speedial entries for this phone. Current algorithm is to find the user
     * associated with the first line, find the groups for that user and see if they are in any of
     * the phone book consumer user groups.
     *
     * @return empty collection of no entries
     */
    Collection<PhonebookEntry> getPhonebookEntries(Phone phone);

    SpeedDial getSpeedDial(Phone phone);

    Collection<Phone> getPhonesByUserIdAndPhoneModel(Integer userId, String modelId);

    User createSpecialPhoneProvisionUser(String serialNumber);

    int getPhonesWithNoLinesCount();

    List<Phone> loadPhonesWithNoLinesByPage(int firstRow, int pageSize, String[] orderBy, boolean orderAscending);
}
