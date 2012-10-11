package org.sipfoundry.sipxconfig.phone.polycom;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Group;

public class FirmwareUpdateListener implements DaoEventListener {
    public PhoneContext m_phoneContext;

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    @Override
    public void onDelete(Object entity) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof Group) {
            Group g = (Group) entity;
            if (Phone.GROUP_RESOURCE_ID.equals(g.getResource())) {
                if (g.getSettingValue("group.version/firmware.version") != null) {
                    for (Phone phone : m_phoneContext.getPhonesByGroupId(g.getId())) {
                        if (phone instanceof PolycomPhone) {
                            phone.setDeviceVersion(DeviceVersion.getDeviceVersion(PolycomPhone.BEAN_ID
                                    + g.getSettingValue("group.version/firmware.version")));
                            m_phoneContext.storePhone(phone);
                        }
                    }
                }
            }
        }

    }

}
