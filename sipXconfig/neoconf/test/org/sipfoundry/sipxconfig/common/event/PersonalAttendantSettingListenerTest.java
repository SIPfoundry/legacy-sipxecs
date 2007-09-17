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
    
    private static final String OPERATOR = "operator";
    
    private PersonalAttendantSettingListener m_out;
    private CoreContext m_coreContextMock;
    private MailboxManager m_mailboxManagerMock;
    private User m_user;
    private Group m_group;
    private PersonalAttendant m_personalAttendant;
    
    public void setUp() {
        Setting userSetting = new SettingSet("user");
        Setting paSetting = new SettingSet("personal-attendant");
        userSetting.addSetting(paSetting);
        Setting operatorSetting = new SettingImpl(OPERATOR);
        operatorSetting.setValue(OPERATOR);
        paSetting.addSetting(operatorSetting);
        
        m_group = new Group();
        m_group.setSettingValue("personal-attendant/operator", OPERATOR);
        
        m_user = new User();
        m_user.setSettings(userSetting);
        m_user.addGroup(m_group);
        
        m_personalAttendant = new PersonalAttendant();
        
        m_coreContextMock = EasyMock.createStrictMock(CoreContext.class);
        m_coreContextMock.getGroupMembers(m_group);
        EasyMock.expectLastCall().andReturn(Collections.<User>singletonList(m_user));
        EasyMock.replay(m_coreContextMock);
        
        m_mailboxManagerMock = EasyMock.createStrictMock(MailboxManager.class);
        m_mailboxManagerMock.loadPersonalAttendantForUser(m_user);
        EasyMock.expectLastCall().andReturn(m_personalAttendant);
        m_mailboxManagerMock.storePersonalAttendant(m_personalAttendant);
        EasyMock.replay(m_mailboxManagerMock);
        
        m_out = new PersonalAttendantSettingListener();
        m_out.setCoreContext(m_coreContextMock);
        m_out.setMailboxManager(m_mailboxManagerMock);
    }
    
    public void testOnSaveGroup() {
        m_out.onSave(m_group);
        assertEquals(OPERATOR, m_personalAttendant.getOperator());
    }
    
    public void testOnSaveUserWithSettings() {
        m_out.onSave(m_user);
        assertEquals(OPERATOR, m_personalAttendant.getOperator());
    }
    
    public void testOnSaveUserWithoutSettings() {
        User userWithoutSettings = new User();
        userWithoutSettings.addGroup(m_group);
        
        m_out.onSave(userWithoutSettings);
        assertEquals(OPERATOR, m_personalAttendant.getOperator());
    }
}
