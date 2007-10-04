package org.sipfoundry.sipxconfig.site.user_portal;

import java.util.Collection;
import java.util.Collections;

import junit.framework.TestCase;

import org.apache.hivemind.Messages;
import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.test.PhonebookTestHelper;

public class UserPhonebookPageTest extends TestCase {
    
    private static final int PORTAL_USER_ID = 1;
    private Creator m_pageCreator;
    private User m_portalUser;
    private CoreContext m_coreContext;
    private PhonebookManager m_phonebookManager;
    private Collection<PhonebookEntry> m_allEntries;
    private UserPhonebookPage m_out;
    private PhonebookTestHelper m_testHelper;

    public void setUp() {
        m_portalUser = new User();
        m_portalUser.setName("portalUser");
        m_testHelper = new PhonebookTestHelper();
        m_allEntries = m_testHelper.getPhonebookEntries();
        
        m_coreContext = EasyMock.createMock(CoreContext.class);
        m_testHelper.configureCoreContextMock(m_coreContext);
        
        m_coreContext.loadUser(PORTAL_USER_ID);
        EasyMock.expectLastCall().andReturn(m_portalUser).anyTimes();
        EasyMock.replay(m_coreContext);
        

        Collection<Phonebook> phonebooks = Collections.singletonList(new Phonebook());
        m_phonebookManager = EasyMock.createMock(PhonebookManager.class);
        m_phonebookManager.getPhonebooksByUser(m_portalUser);
        EasyMock.expectLastCall().andReturn(phonebooks).anyTimes();
        m_phonebookManager.getEntries(phonebooks);
        EasyMock.expectLastCall().andReturn(m_allEntries).anyTimes();
        EasyMock.replay(m_phonebookManager);
        
        m_pageCreator = new Creator();
        m_out = (UserPhonebookPage)m_pageCreator.newInstance(UserPhonebookPage.class);
        PropertyUtils.write(m_out, "coreContext", m_coreContext);
        PropertyUtils.write(m_out, "phonebookManager", m_phonebookManager);
        
        Messages messages = EasyMock.createMock(Messages.class);
        messages.getMessage(EasyMock.isA(String.class));
        EasyMock.expectLastCall().andReturn("").anyTimes();
        EasyMock.replay(messages);
        
        PropertyUtils.write(m_out, "messages", messages);
        m_out.setUserId(1);
        m_out.setPhonebookEntries(m_allEntries);
    }
    
    public void testGetExtensionsForCurrentEntry() {
        for (PhonebookEntry entry : m_allEntries) {
            m_out.setPhonebookEntry(entry);
            String extensionString = m_out.getExtensionsForCurrentEntry();
            for (String extension : m_testHelper.getExtensions(entry.getNumber())) {
                assertTrue(extensionString.contains(extension));
            }
        }
    }
    
    public void testGetSipIdsForCurrentEntry() {
        for (PhonebookEntry entry : m_allEntries) {
            m_out.setPhonebookEntry(entry);
            String sipIdString = m_out.getSipIdsForCurrentEntry();
            for (String sipId : m_testHelper.getSipIds(entry.getNumber())) {
                assertTrue(sipIdString.contains(sipId));
            }
        }
    }
}
