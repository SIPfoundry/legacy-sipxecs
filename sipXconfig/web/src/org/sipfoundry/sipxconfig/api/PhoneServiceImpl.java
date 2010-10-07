/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.rmi.RemoteException;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.device.RestartManager;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public class PhoneServiceImpl implements PhoneService {

    private static final String GROUP_RESOURCE_ID = org.sipfoundry.sipxconfig.phone.Phone.GROUP_RESOURCE_ID;

    private PhoneContext m_context;

    private SettingDao m_settingDao;

    private PhoneBuilder m_builder;

    private CoreContext m_coreContext;

    private ProfileManager m_profileManager;

    private ModelSource<PhoneModel> m_phoneModelSource;

    private RestartManager m_restartManager;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setPhoneContext(PhoneContext context) {
        m_context = context;
    }

    public void setPhoneBuilder(PhoneBuilder builder) {
        m_builder = builder;
    }

    public void setProfileManager(ProfileManager profileManager) {
        m_profileManager = profileManager;
    }

    public void setRestartManager(RestartManager restartManager) {
        m_restartManager = restartManager;
    }

    public void addPhone(AddPhone addPhone) throws RemoteException {
        Phone apiPhone = addPhone.getPhone();
        PhoneModel model = requireModelId(apiPhone.getModelId());
        org.sipfoundry.sipxconfig.phone.Phone phone = m_context.newPhone(model);
        ApiBeanUtil.toMyObject(m_builder, phone, apiPhone);
        String[] groups = apiPhone.getGroups();
        for (int i = 0; groups != null && i < groups.length; i++) {
            Group g = m_settingDao.getGroupCreateIfNotFound(GROUP_RESOURCE_ID, groups[i]);
            phone.addGroup(g);
        }
        m_context.storePhone(phone);
    }

    public FindPhoneResponse findPhone(FindPhone findPhone) throws RemoteException {
        FindPhoneResponse response = new FindPhoneResponse();
        org.sipfoundry.sipxconfig.phone.Phone[] myPhones = phoneSearch(findPhone.getSearch());
        Phone[] arrayOfPhones = (Phone[]) ApiBeanUtil.toApiArray(m_builder, myPhones, Phone.class);
        response.setPhones(arrayOfPhones);

        return response;
    }

    org.sipfoundry.sipxconfig.phone.Phone[] phoneSearch(PhoneSearch search) {
        Collection phones = Collections.EMPTY_LIST;
        if (search == null) {
            phones = m_context.loadPhones();
        } else if (search.getBySerialNumber() != null) {
            // we do not know which model we are looking for - the best we can do is to clean
            // serial number with default model
            PhoneModel model = new PhoneModel();
            String serialNo = model.cleanSerialNumber(search.getBySerialNumber());
            Integer id = m_context.getPhoneIdBySerialNumber(serialNo);
            if (id != null) {
                org.sipfoundry.sipxconfig.phone.Phone phone = m_context.loadPhone(id);
                if (phone != null) {
                    phones = Collections.singleton(phone);
                }
            }
        } else if (search.getByGroup() != null) {
            Group g = m_settingDao.getGroupByName(GROUP_RESOURCE_ID, search.getByGroup());
            if (g != null) {
                phones = m_context.getPhonesByGroupId(g.getId());
            }
        }

        return (org.sipfoundry.sipxconfig.phone.Phone[]) phones
                .toArray(new org.sipfoundry.sipxconfig.phone.Phone[phones.size()]);
    }

    public PhoneModel requireModelId(String modelId) {
        PhoneModel model = m_phoneModelSource.getModel(modelId);
        if (model == null) {
            throw new IllegalArgumentException("phone model doesn't exist: " + modelId);
        }
        return model;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public void managePhone(ManagePhone managePhone) throws RemoteException {
        org.sipfoundry.sipxconfig.phone.Phone[] myPhones = phoneSearch(managePhone.getSearch());
        Collection ids = CollectionUtils.collect(Arrays.asList(myPhones), new BeanWithId.BeanToId());
        if (Boolean.TRUE.equals(managePhone.getGenerateProfiles())) {
            m_profileManager.generateProfiles(ids, false, null);
        } else if (Boolean.TRUE.equals(managePhone.getRestart())) {
            m_restartManager.restart(ids, null);
        } else {
            for (int i = 0; i < myPhones.length; i++) {

                if (Boolean.TRUE.equals(managePhone.getDeletePhone())) {
                    m_context.deletePhone(myPhones[i]);
                    continue; // all other edits wouldn't make sense
                }

                if (managePhone.getEdit() != null) {
                    Phone apiPhone = new Phone();
                    Set properties = ApiBeanUtil.getSpecifiedProperties(managePhone.getEdit());
                    ApiBeanUtil.setProperties(apiPhone, managePhone.getEdit());
                    m_builder.toMyObject(myPhones[i], apiPhone, properties);
                    m_context.storePhone(myPhones[i]);
                }

                if (managePhone.getRemoveLineByUserId() != null) {
                    String username = managePhone.getRemoveLineByUserId();
                    org.sipfoundry.sipxconfig.phone.Line l = myPhones[i].findByUsername(username);
                    if (l != null) {
                        m_context.deleteLine(l);
                    }
                }

                if (managePhone.getRemoveLineByUri() != null) {
                    String uri = managePhone.getRemoveLineByUri();
                    org.sipfoundry.sipxconfig.phone.Line l = myPhones[i].findByUri(uri);
                    if (l != null) {
                        m_context.deleteLine(l);
                    }
                }

                if (managePhone.getAddLine() != null) {
                    String userName = managePhone.getAddLine().getUserId();
                    org.sipfoundry.sipxconfig.common.User u = m_coreContext.loadUserByUserName(userName);
                    org.sipfoundry.sipxconfig.phone.Line l = myPhones[i].createLine();
                    l.setUser(u);
                    myPhones[i].addLine(l);
                    m_context.storePhone(myPhones[i]);
                }

                if (managePhone.getAddExternalLine() != null) {
                    AddExternalLine eline = managePhone.getAddExternalLine();
                    org.sipfoundry.sipxconfig.phone.Line l = myPhones[i].createLine();
                    myPhones[i].addLine(l);
                    LineInfo einfo = new LineInfo();
                    ApiBeanUtil.toMyObject(new SimpleBeanBuilder(), einfo, eline);
                    l.setLineInfo(einfo);
                    m_context.storePhone(myPhones[i]);
                }

                if (managePhone.getAddGroup() != null) {
                    Group g = m_settingDao.getGroupCreateIfNotFound(GROUP_RESOURCE_ID, managePhone
                            .getAddGroup());
                    myPhones[i].addGroup(g);
                    m_context.storePhone(myPhones[i]);
                }

                if (managePhone.getRemoveGroup() != null) {
                    Group g = m_settingDao.getGroupByName(GROUP_RESOURCE_ID, managePhone.getRemoveGroup());
                    if (g != null) {
                        DataCollectionUtil.removeByPrimaryKey(myPhones[i].getGroups(), g.getPrimaryKey());
                    }
                    m_context.storePhone(myPhones[i]);
                }
            }
        }
    }

    public void setPhoneModelSource(ModelSource<PhoneModel> phoneModelSource) {
        m_phoneModelSource = phoneModelSource;
    }
}
