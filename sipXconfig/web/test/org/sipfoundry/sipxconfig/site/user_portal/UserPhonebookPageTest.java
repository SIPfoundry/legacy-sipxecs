package org.sipfoundry.sipxconfig.site.user_portal;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

import junit.framework.TestCase;

import org.apache.hivemind.util.PropertyUtils;
import org.apache.lucene.queryParser.ParseException;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;

public class UserPhonebookPageTest extends TestCase {
    
    private Creator m_pageCreator;
    private User m_portalUser;
    private User m_firstUser;
    private User m_secondUser;
    private PhonebookEntry m_firstEntry;
    private PhonebookEntry m_secondEntry;
    private PhonebookEntry m_thirdEntry;
    private PhonebookEntry m_fourthEntry;
    private CoreContext m_coreContext;
    private PhonebookManager m_phonebookManager;
    private Collection<PhonebookEntry> m_allEntries;
    private UserPhonebookPage m_out;

    public void setUp() {
        m_portalUser = new User();
        m_portalUser.setName("portalUser");
        
        m_firstUser = new User();
        m_firstUser.setName("firstUser");
        m_firstUser.setFirstName("first");
        m_firstUser.setLastName("user");
        m_firstUser.setAliasesString("200 anotherUserName 501");
        
        m_secondUser = new User();
        m_secondUser.setName("300");
        m_secondUser.setFirstName("second");
        m_secondUser.setLastName("user");
        m_secondUser.setAliasesString("secondUser 310");
        
        m_firstEntry = new PhonebookEntry() {
            public String getNumber() {
                return "firstUser";
            }
            public String getLastName() {
                return "user";
            }
            public String getFirstName() {
                return "first";
            }
        };
        
        m_secondEntry = new PhonebookEntry() {
            public String getNumber() {
                return "300";
            }
            public String getLastName() {
                return "user";
            }
            public String getFirstName() {
                return "second";
            }
        };
        
        m_thirdEntry = new PhonebookEntry() {
            public String getNumber() {
                return "700";
            }
            public String getLastName() {
                return "other";
            }
            public String getFirstName() {
                return "third";
            }
        };
        
        m_fourthEntry = new PhonebookEntry() {
            public String getNumber() {
                return "fourthUser";
            }
            public String getLastName() {
                return "user";
            }
            public String getFirstName() {
                return "fourth";
            }
        };
        
        m_coreContext = EasyMock.createMock(CoreContext.class);
        m_coreContext.loadUserByUserName(m_firstEntry.getNumber());
        EasyMock.expectLastCall().andReturn(m_firstUser).anyTimes();
        m_coreContext.loadUserByUserName(m_secondEntry.getNumber());
        EasyMock.expectLastCall().andReturn(m_secondUser).anyTimes();
        m_coreContext.loadUserByUserName(m_thirdEntry.getNumber());
        EasyMock.expectLastCall().andReturn(null).anyTimes();
        m_coreContext.loadUserByUserName(m_fourthEntry.getNumber());
        EasyMock.expectLastCall().andReturn(null).anyTimes();
        m_coreContext.loadUser(1);
        EasyMock.expectLastCall().andReturn(m_portalUser).anyTimes();
        EasyMock.replay(m_coreContext);
        
        m_allEntries = new ArrayList<PhonebookEntry>();
        m_allEntries.add(m_firstEntry);
        m_allEntries.add(m_secondEntry);
        m_allEntries.add(m_thirdEntry);
        m_allEntries.add(m_fourthEntry);

        Collection<Phonebook> phonebooks = Collections.singletonList(new Phonebook());
        m_phonebookManager = EasyMock.createMock(PhonebookManager.class);
        m_phonebookManager.getPhonebooksByUser(m_portalUser);
        EasyMock.expectLastCall().andReturn(phonebooks).anyTimes();
        m_phonebookManager.getRows(phonebooks);
        EasyMock.expectLastCall().andReturn(m_allEntries).anyTimes();
        EasyMock.replay(m_phonebookManager);
        
        m_pageCreator = new Creator();
        m_out = (UserPhonebookPage)m_pageCreator.newInstance(UserPhonebookPage.class);
        PropertyUtils.write(m_out, "coreContext", m_coreContext);
        PropertyUtils.write(m_out, "phonebookManager", m_phonebookManager);
        m_out.setUserId(1);
        m_out.setPhonebookEntries(m_allEntries);
    }
    
    public void testGetExtensionsForCurrentEntry() {
        m_out.setPhonebookEntry(m_firstEntry);
        String firstExtensionString = m_out.getExtensionsForCurrentEntry();
        assertTrue(firstExtensionString.contains("200"));
        assertTrue(firstExtensionString.contains("501"));
        
        m_out.setPhonebookEntry(m_secondEntry);
        String secondExtensionString = m_out.getExtensionsForCurrentEntry();
        assertTrue(secondExtensionString.contains("300"));
        assertTrue(secondExtensionString.contains("310"));

        m_out.setPhonebookEntry(m_thirdEntry);
        String thirdExtensionString = m_out.getExtensionsForCurrentEntry();
        assertTrue(thirdExtensionString.contains("700"));
    }
    
    public void testGetSipIdsForCurrentEntry() {
        m_out.setPhonebookEntry(m_firstEntry);
        String firstSipIdString = m_out.getSipIdsForCurrentEntry();
        assertTrue(firstSipIdString.contains("firstUser"));
        assertTrue(firstSipIdString.contains("anotherUserName"));
        
        m_out.setPhonebookEntry(m_secondEntry);
        String secondSipIdString = m_out.getSipIdsForCurrentEntry();
        assertTrue(secondSipIdString.contains("secondUser"));

        m_out.setPhonebookEntry(m_thirdEntry);
        String thirdSipIdString = m_out.getSipIdsForCurrentEntry();
        assertTrue(thirdSipIdString.contains("LABEL.UNKNOWN"));
        
        m_out.setPhonebookEntry(m_fourthEntry);
        String fourthSipIdString = m_out.getSipIdsForCurrentEntry();
        assertTrue(fourthSipIdString.contains("fourthUser"));
    }
    
    public void testSearch() throws IOException, ParseException {
        // should only match a single entry
        m_out.setQuery("first");
        m_out.search();
        Collection<PhonebookEntry> entriesThatMatchFirstUser = m_out.getPhonebookEntries();
        assertEquals(1, entriesThatMatchFirstUser.size());
        assertTrue(entriesThatMatchFirstUser.contains(m_firstEntry));
        
        // should only match a single entry
        m_out.reset();
        m_out.setQuery("second");
        m_out.search();
        Collection<PhonebookEntry> entriesThatMatchSecondUser = m_out.getPhonebookEntries();
        assertEquals(1, entriesThatMatchSecondUser.size());
        assertTrue(entriesThatMatchSecondUser.contains(m_secondEntry));
        
        // should match all entries except for the entry with number = 700
        m_out.reset();
        m_out.setQuery("user");
        m_out.search();
        Collection<PhonebookEntry> entriesThatMatchUser = m_out.getPhonebookEntries();
        assertEquals(3, entriesThatMatchUser.size());
        assertTrue(entriesThatMatchUser.contains(m_firstEntry));
        assertTrue(entriesThatMatchUser.contains(m_secondEntry));
        assertTrue(entriesThatMatchUser.contains(m_fourthEntry));
        
    }
}
