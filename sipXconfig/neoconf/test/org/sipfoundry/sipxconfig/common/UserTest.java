/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.common;

import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;

public class UserTest extends TestCase {

    public void testGetDisplayName() {
        User u = new User();
        assertNull(u.getDisplayName());
        u.setUserName("bob");
        assertNull(u.getDisplayName());
        u.setFirstName("First");
        assertEquals("First", u.getDisplayName());
        u.setLastName("Last");
        assertEquals("First Last", u.getDisplayName());
    }

    public void testGetLabel() {
        User u = new User();
        assertEquals("", u.getLabel());

        u.setUserName("bob");
        assertNull(u.getDisplayName());
        assertEquals("bob", u.getLabel());
        u.setFirstName("First");
        assertEquals("First", u.getLabel());
        u.setLastName("Last");
        assertEquals("First Last", u.getLabel());
    }

    public void testGetUri() {
        User user = new User();
        user.setUserName("username");
        String uri = user.getUri("mycomp.com");

        assertEquals("sip:username@mycomp.com", uri);

        user.setLastName("Last");
        uri = user.getUri("mycomp.com");
        assertEquals("\"Last\"<sip:username@mycomp.com>", uri);

        user.setFirstName("First");
        uri = user.getUri("mycomp.com");
        assertEquals("\"First Last\"<sip:username@mycomp.com>", uri);
    }

    public void testGetAddrSpec() {
        User user = new User();
        user.setUserName("username");
        String uri = user.getAddrSpec("mycomp.com");

        assertEquals("sip:username@mycomp.com", uri);

        user.setLastName("Last");
        user.setFirstName("First");
        uri = user.getAddrSpec("mycomp.com");
        assertEquals("sip:username@mycomp.com", uri);
    }

    /** Test that setting a typical PIN yields expected results */
    public void testSetPin() throws Exception {
        checkSetPin("pin");
    }

    /** Test that setting a null PIN yields expected results */
    public void testSetNullPin() throws Exception {
        checkSetPin(null);
    }

    private void checkSetPin(String pin) throws Exception {
        User user = new User();
        user.setUserName("username");
        user.setPin(pin, "realm.sipfoundry.org");
        String pintoken = getPintoken("username", pin);
        assertEquals(pintoken, user.getPintoken());
    }

    public void testGetSipPasswordHash() throws Exception {
        User user = new User();
        user.setUserName("username");
        user.setSipPassword("sip password");
        String hash = Md5Encoder.digestPassword("username", "realm.sipfoundry.org", "sip password");

        assertEquals(hash, user.getSipPasswordHash("realm.sipfoundry.org"));
    }

    public void testGetSipPasswordHashEmpty() throws Exception {
        User user = new User();
        user.setUserName("username");
        user.setSipPassword(null);
        String hash = Md5Encoder.digestPassword("username", "realm.sipfoundry.org", "");

        assertEquals(hash, user.getSipPasswordHash("realm.sipfoundry.org"));
    }

    public void testGetSipPasswordHashMd5() throws Exception {
        User user = new User();
        user.setUserName("username");
        String hash = Md5Encoder.digestPassword("username", "realm.sipfoundry.org", "");
        user.setSipPassword(hash);

        String newHash = Md5Encoder.digestPassword("username", "realm.sipfoundry.org", hash);

        assertFalse(hash.equals(newHash));
        assertEquals(newHash, user.getSipPasswordHash("realm.sipfoundry.org"));
    }

    public void testGetAliasesStringSorted() {
        User user = new User();
        user.setUserName("username");

        Set aliases = new LinkedHashSet(); // use LinkedHashSet for stable ordering
        aliases.add("mambo");
        aliases.add("tango");
        aliases.add("django");
        user.setAliases(aliases);
        assertEquals("django mambo tango", user.getAliasesString());
    }

    public void testGetAliases() {
        User user = new User() {
            @Override
            public String getFaxExtension() {
                return new String("321");
            }

            @Override
            public String getFaxDid() {
                return new String("+1234");
            }
        };
        user.setUserName("username");

        Set aliases = new LinkedHashSet(); // use LinkedHashSet for stable ordering
        aliases.add("mambo");
        aliases.add("tango");
        user.setAliases(aliases);
        assertEquals("mambo tango", user.getAliasesString());
        checkAliases(user);

        user.setAliases(new LinkedHashSet());
        user.setAliasesString("mambo tango");
        checkAliases(user);

        List aliasMappings = user.getAliasMappings("sipfoundry.org", "imId");
        assertEquals(5, aliasMappings.size());

        AliasMapping alias = (AliasMapping) aliasMappings.get(0);
        assertEquals("mambo@sipfoundry.org", alias.getIdentity());

        final String CONTACT = "sip:username@sipfoundry.org";
        assertEquals(CONTACT, alias.getContact());
        alias = (AliasMapping) aliasMappings.get(1);
        assertEquals("tango@sipfoundry.org", alias.getIdentity());
        assertEquals(CONTACT, alias.getContact());

        AliasMapping imIdAlias = (AliasMapping) aliasMappings.get(2);
        assertEquals("imId@sipfoundry.org", imIdAlias.getIdentity());

        AliasMapping faxAlias = (AliasMapping) aliasMappings.get(3);
        assertEquals("321@sipfoundry.org", faxAlias.getIdentity());
        assertEquals("sip:~~ff~" + user.getUserName() + "@sipfoundry.org", faxAlias.getContact());

        List otherAliasMappings1 = user.getAliasMappings("sipfoundry.org", "mambo");
        assertEquals(4, otherAliasMappings1.size());

        // Set the additional alias, imId, to user's userName, it should not be
        // added as an alias.
        String imId = user.getUserName();
        List otherAliasMappings2 = user.getAliasMappings("sipfoundry.org", imId);
        assertEquals(4, otherAliasMappings2.size());
    }

    private void checkAliases(User user) {
        Set aliasesCheck = user.getAliases();
        assertEquals(2, aliasesCheck.size());
        Iterator i = aliasesCheck.iterator();
        assertEquals("mambo", i.next());
        assertEquals("tango", i.next());
    }

    public void testGetEmptyAliases() {
        User user = new User() {
            @Override
            public String getFaxExtension() {
                return "";
            }
            @Override
            public String getFaxDid() {
                return new String("");
            }
        };
        user.setUserName("username");
        List aliasMappings = user.getAliasMappings("sipfoundry.org", "");
        assertEquals(0, aliasMappings.size());
    }

    public void testHasPermission() {
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User user = new User();
        user.setPermissionManager(pm);

        Group group = new Group();
        user.addGroup(group);

        String path = PermissionName.SUPERADMIN.getPath();
        Setting superAdmin = user.getSettings().getSetting(path);
        assertNotNull(superAdmin);
        assertFalse(user.hasPermission(PermissionName.SUPERADMIN));
    }

    public void testSetPermission() {
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User user = new User();
        user.setPermissionManager(pm);

        assertFalse(user.isAdmin());
        user.setPermission(PermissionName.SUPERADMIN, true);
        assertTrue(user.isAdmin());
    }

    public void testGetExtension() throws Exception {
        User user = new User();
        user.setUserName("abc");

        assertNull(user.getExtension(true));

        user.addAliases(new String[] {
            "5a", "oooi", "333", "xyz", "4444"
        });
        assertEquals("333", user.getExtension(true));
        assertFalse(user.hasNumericUsername());

        user.addAlias("2344");
        assertEquals("333", user.getExtension(true));

        user.addAlias("01");
        assertEquals("01", user.getExtension(true));

        user.setUserName("12345");
        assertEquals("12345", user.getExtension(true));
        assertTrue(user.hasNumericUsername());

        user.setUserName("12345");
        assertEquals("01", user.getExtension(false));

        // make sure 0 is ignored as possible extension
        user.addAlias("0");
        assertEquals("01", user.getExtension(false));
    }

    public void testGetSite() {
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User user = new User();
        user.setPermissionManager(pm);

        assertNull(user.getSite());

        Group madridGroup = new Group();
        madridGroup.setUniqueId();
        madridGroup.setWeight(100);
        user.addGroup(madridGroup);

        Branch madrid = new Branch();
        madridGroup.setBranch(madrid);

        assertSame(madrid, user.getSite());

        Group lisbonGroup = new Group();
        lisbonGroup.setUniqueId();
        lisbonGroup.setWeight(50);
        Branch lisbon = new Branch();
        lisbonGroup.setBranch(lisbon);

        user.addGroup(lisbonGroup);

        assertSame(lisbon, user.getSite());

        user.removeGroup(lisbonGroup);
        lisbonGroup.setWeight(150);
        user.addGroup(lisbonGroup);

        assertSame(madrid, user.getSite());

        user.setBranch(lisbon);
        assertSame(lisbon, user.getSite());
    }

    public void testGetMusicOnHoldUri() {
        MusicOnHoldManager musicOnHoldManager = createMock(MusicOnHoldManager.class);
        musicOnHoldManager.getDefaultMohUri();
        expectLastCall().andReturn("sip:~~mh@example.org").anyTimes();
        musicOnHoldManager.getLocalFilesMohUri();
        expectLastCall().andReturn("sip:~~mh~l@example.org").anyTimes();
        musicOnHoldManager.getPersonalMohFilesUri("1234");
        expectLastCall().andReturn("sip:~~mh~1234@example.org").anyTimes();
        musicOnHoldManager.getPortAudioMohUri();
        expectLastCall().andReturn("sip:~~mh~p@example.org").anyTimes();
        musicOnHoldManager.getNoneMohUri();
        expectLastCall().andReturn("sip:~~mh~n@example.org").anyTimes();
        replay(musicOnHoldManager);

        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User user = new User();
        user.setPermissionManager(pm);
        user.setMusicOnHoldManager(musicOnHoldManager);
        user.setName("1234");

        assertEquals("sip:~~mh@example.org", user.getMusicOnHoldUri());

        user.setSettingValue(User.MOH_AUDIO_SOURCE_SETTING, User.MohAudioSource.FILES_SRC.toString());
        assertEquals("sip:~~mh~l@example.org", user.getMusicOnHoldUri());

        user.setSettingValue(User.MOH_AUDIO_SOURCE_SETTING, User.MohAudioSource.PERSONAL_FILES_SRC.toString());
        assertEquals("sip:~~mh~1234@example.org", user.getMusicOnHoldUri());

        user.setSettingValue(User.MOH_AUDIO_SOURCE_SETTING, User.MohAudioSource.SOUNDCARD_SRC.toString());
        assertEquals("sip:~~mh~p@example.org", user.getMusicOnHoldUri());

        user.setSettingValue(User.MOH_AUDIO_SOURCE_SETTING, User.MohAudioSource.NONE.toString());
        assertEquals("sip:~~mh~n@example.org", user.getMusicOnHoldUri());

    }

    private String getPintoken(String username, String pin) {
        // handle null pin
        String safePin = StringUtils.defaultString(pin);
        return Md5Encoder.digestPassword(username, "realm.sipfoundry.org", safePin);
    }
}
