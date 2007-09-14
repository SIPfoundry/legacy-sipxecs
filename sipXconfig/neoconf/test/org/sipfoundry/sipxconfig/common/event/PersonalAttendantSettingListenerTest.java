package org.sipfoundry.sipxconfig.common.event;

import java.util.Collections;

import junit.framework.TestCase;

import org.easymock.classextension.EasyMock;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;

public class PersonalAttendantSettingListenerTest extends TestCase {
    public void testOnSave() {
        Setting userSetting = new SettingSet("user");
        Setting paSetting = new SettingSet("personal-attendant");
        userSetting.addSetting(paSetting);
        Setting operatorSetting = new SettingImpl("operator");
        operatorSetting.setValue("operator");
        paSetting.addSetting(operatorSetting);
        
        User user = new User();
        user.setSettings(userSetting);
        
        Group group = new Group();
        
        CoreContext coreContextMock = EasyMock.createStrictMock(CoreContext.class);
        coreContextMock.getGroupMembers(group);
        EasyMock.expectLastCall().andReturn(Collections.<User>singletonList(user));
        EasyMock.replay(coreContextMock);
        
        PersonalAttendant personalAttendant = new PersonalAttendant();
        
        MailboxManager mailboxManagerMock = EasyMock.createStrictMock(MailboxManager.class);
        mailboxManagerMock.loadPersonalAttendantForUser(user);
        EasyMock.expectLastCall().andReturn(personalAttendant);
        mailboxManagerMock.storePersonalAttendant(personalAttendant);
        EasyMock.replay(mailboxManagerMock);
        
        PersonalAttendantSettingListener out = new PersonalAttendantSettingListener();
        out.setCoreContext(coreContextMock);
        out.setMailboxManager(mailboxManagerMock);
        
        out.onSave(group);
        
        assertEquals("operator", personalAttendant.getOperator());
    }
}
