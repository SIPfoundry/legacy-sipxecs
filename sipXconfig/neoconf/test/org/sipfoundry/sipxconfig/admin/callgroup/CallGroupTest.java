/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.callgroup;

import java.util.List;
import java.util.Scanner;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.User;

public class CallGroupTest extends TestCase {

    public void testInsertRingUser() {
        User u = new User();
        u.setUserName("testUser");
        CallGroup group = new CallGroup();
        UserRing ring = group.insertRingForUser(u);
        assertSame(u, ring.getUser());
        List calls = group.getRings();
        assertEquals(1, calls.size());
        assertSame(ring, calls.get(0));
        assertTrue(ring.isFirst());
    }

    public void testGenerateAliases() {
        int numRings = 5;
        CallGroup group = createCallGroupWithUsers("401", "sales", numRings);

        List<AliasMapping> aliases = group.generateAliases("kuku");
        group.setEnabled(true);
        aliases = group.generateAliases("kuku");

        assertEquals(numRings + 1, aliases.size());
        for (int i = 0; i < numRings; i++) {
            AliasMapping am = aliases.get(i);
            assertEquals(am.getIdentity(), group.getName() + "@kuku");
            assertTrue(am.getContact().startsWith("<sip:testUser" + i + "@kuku"));
            // all of the contacts should contain sipx-noroute=Voicemail
            assertTrue(am.getContact().contains("sipx-noroute=Voicemail"));
        }

        // the last alias is an extension => identity
        AliasMapping am = aliases.get(aliases.size() - 1);
        assertEquals(am.getIdentity(), group.getExtension() + "@kuku");
        assertTrue(am.getContact().startsWith(group.getName() + "@kuku"));
    }

    public void testGenerateAliasesForDisabledGroup() {
        int numRings = 5;
        CallGroup group = createCallGroupWithUsers("401", "sales", numRings);

        List<AliasMapping> aliases = group.generateAliases("kuku");
        // disabled group should not generate aliases
        assertTrue(aliases.isEmpty());

        // not even when fallback is enabled
        group.setVoicemailFallback(true);
        assertTrue(aliases.isEmpty());

        group.setFallbackDestination("fallback@kuku.com");
        assertTrue(aliases.isEmpty());

        group.setUserForward(true);
        assertTrue(aliases.isEmpty());
    }

    public void testGenerateAliasesWithVoicemailFallback() {
        int numRings = 5;
        CallGroup group = createCallGroupWithUsers("401", "sales", numRings);
        group.setVoicemailFallback(true);
        group.setEnabled(true);

        // assumption: the aliases list is ordered from highest q value to lowest,
        // with last element being the identity alias.
        List<AliasMapping> aliases = group.generateAliases("kuku");

        assertEquals(numRings + 2, aliases.size());
        double lastQ = 1;
        for (int i = 0; i < numRings; i++) {
            AliasMapping am = aliases.get(i);
            assertEquals(am.getIdentity(), group.getName() + "@kuku");
            String contact = am.getContact();
            assertTrue(contact.startsWith("<sip:testUser" + i + "@kuku"));
            // all of the contacts should contain sipx-noroute=Voicemail
            assertTrue(contact.contains("sipx-noroute=Voicemail"));
            double q = parseQ(contact);
            assertTrue(q < lastQ);
            assertTrue(q > 0.8);
            lastQ = q;
        }

        // the second to last alias (lowest q value) should sent last user to voicemail
        AliasMapping vmailMapping = aliases.get(aliases.size() - 2);
        assertEquals(group.getName() + "@kuku", vmailMapping.getIdentity());
        String contact = vmailMapping.getContact();
        assertTrue(contact.startsWith("<sip:~~vm~testUser4@kuku"));
        double q = parseQ(contact);
        assertTrue(q < lastQ);

        // the last alias is an extension => identity
        AliasMapping am = aliases.get(aliases.size() - 1);
        assertEquals(am.getIdentity(), group.getExtension() + "@kuku");
        assertTrue(am.getContact().startsWith(group.getName() + "@kuku"));
    }

    public void testGenerateAliasesWithVoicemailFallbackParallelGroup() {
        int numRings = 3;
        CallGroup group = createCallGroupWithUsers("401", "sales", numRings);
        group.setVoicemailFallback(true);
        group.setEnabled(true);
        List<AbstractRing> rings = group.getRings();
        for (AbstractRing ring : rings) {
            ring.setType(AbstractRing.Type.IMMEDIATE);
        }

        // assumption: the aliases list is ordered from highest q value to lowest,
        // with last element being the identity alias.
        List<AliasMapping> aliases = group.generateAliases("kuku");

        assertEquals(numRings + 2, aliases.size());
        double lastQ = 0;
        for (int i = 0; i < numRings; i++) {
            AliasMapping am = aliases.get(i);
            assertEquals(am.getIdentity(), group.getName() + "@kuku");
            String contact = am.getContact();
            assertTrue(contact.startsWith("<sip:testUser" + i + "@kuku"));
            // all of the contacts should contain sipx-noroute=Voicemail
            assertTrue(contact.contains("sipx-noroute=Voicemail"));
            double q = parseQ(contact);
            if (lastQ == 0) {
                lastQ = q;
            }
            assertEquals(q, lastQ);
        }

        // the second to last alias (lowest q value) should sent last user to voicemail
        AliasMapping vmailMapping = aliases.get(aliases.size() - 2);
        assertEquals(group.getName() + "@kuku", vmailMapping.getIdentity());
        String contact = vmailMapping.getContact();
        assertTrue(contact.startsWith("<sip:~~vm~testUser2@kuku"));
        double q = parseQ(contact);
        assertTrue(q < lastQ);

        // the last alias is an extension => identity
        AliasMapping am = aliases.get(aliases.size() - 1);
        assertEquals(am.getIdentity(), group.getExtension() + "@kuku");
        assertTrue(am.getContact().startsWith(group.getName() + "@kuku"));
    }

    public void testGenerateAliasesWithUserForwardDisabled() {
        int numRings = 5;
        CallGroup group = createCallGroupWithUsers("401", "sales", numRings);
        group.setUserForward(false);
        group.setEnabled(true);

        // assumption: the aliases list is ordered from highest q value to lowest,
        // with last element being the identity alias.
        List<AliasMapping> aliases = group.generateAliases("kuku");

        assertEquals(numRings + 1, aliases.size());
        for (int i = 0; i < numRings; i++) {
            AliasMapping am = aliases.get(i);
            assertEquals(am.getIdentity(), group.getName() + "@kuku");
            assertTrue(am.getContact().startsWith("<sip:testUser" + i + "@kuku"));
            // all of the contacts should contain sipx-noroute=Voicemail
            assertTrue(am.getContact().contains("sipx-userforward=false"));
        }

    }

    public void testGenerateAliasesForEmptyGroupWithVoicemailFallback() {
        CallGroup group = createCallGroupWithUsers("401", "sales", 0);
        group.setVoicemailFallback(true);
        group.setEnabled(true);

        // assumption: the aliases list is ordered from highest q value to lowest,
        // with last element being the identity alias.
        List<AliasMapping> aliases = group.generateAliases("kuku");

        assertEquals(1, aliases.size());
        // the last alias is an extension => identity
        AliasMapping am = aliases.get(aliases.size() - 1);
        assertEquals(am.getIdentity(), group.getExtension() + "@kuku");
        assertTrue(am.getContact().startsWith(group.getName() + "@kuku"));
    }

    public void testGenerateAliasesWithFallbackDestinationAsSipUri() {
        int numRings = 5;
        CallGroup group = createCallGroupWithUsers("401", "sales", numRings);
        String fallbackDestination = "fallback@kuku.com";
        group.setFallbackDestination(fallbackDestination);
        group.setEnabled(true);

        // assumption: the aliases list is ordered from highest q value to lowest,
        // with last element being the identity alias.
        List<AliasMapping> aliases = group.generateAliases("kuku");

        assertEquals(numRings + 2, aliases.size());
        double lastQ = 1;
        for (int i = 0; i < numRings; i++) {
            AliasMapping am = aliases.get(i);
            assertEquals(am.getIdentity(), group.getName() + "@kuku");
            String contact = am.getContact();
            assertTrue(contact.startsWith("<sip:testUser" + i + "@kuku"));
            // all of the contacts should contain sipx-noroute=Voicemail
            assertTrue(contact.contains("sipx-noroute=Voicemail"));
            double q = parseQ(contact);
            assertTrue(q < lastQ);
            assertTrue(q > 0.8);
            lastQ = q;
        }

        // the second to last alias (lowest q value) should be the default sip uri
        AliasMapping defaultSipUriMapping = aliases.get(aliases.size() - 2);
        assertEquals(group.getName() + "@kuku", defaultSipUriMapping.getIdentity());
        String contact = defaultSipUriMapping.getContact();
        assertTrue(contact.contains("fallback@kuku.com"));
        assertTrue(parseQ(contact) < lastQ);

        // the last alias is an extension => identity
        AliasMapping am = aliases.get(aliases.size() - 1);
        assertEquals(am.getIdentity(), group.getExtension() + "@kuku");
        assertTrue(am.getContact().startsWith(group.getName() + "@kuku"));
    }

    public void testGenerateAliasesWithFallbackDestinationAsExtension() {
        int numRings = 5;
        CallGroup group = createCallGroupWithUsers("401", "sales", numRings);
        String fallbackDestination = "1234";
        group.setFallbackDestination(fallbackDestination);
        group.setEnabled(true);

        // assumption: the aliases list is ordered from highest q value to lowest,
        // with last element being the identity alias.
        List<AliasMapping> aliases = group.generateAliases("kuku");

        assertEquals(numRings + 2, aliases.size());
        for (int i = 0; i < numRings; i++) {
            AliasMapping am = aliases.get(i);
            assertEquals(am.getIdentity(), group.getName() + "@kuku");
            assertTrue(am.getContact().startsWith("<sip:testUser" + i + "@kuku"));
            // all of the contacts should contain sipx-noroute=Voicemail
            // (this is different from case without default sip uri)
            assertTrue(am.getContact().contains("sipx-noroute=Voicemail"));
        }

        // the second to last alias (lowest q value) should be the default sip uri
        AliasMapping defaultSipUriMapping = aliases.get(aliases.size() - 2);
        assertEquals(group.getName() + "@kuku", defaultSipUriMapping.getIdentity());
        assertTrue(defaultSipUriMapping.getContact().contains("1234@kuku"));

        // the last alias is an extension => identity
        AliasMapping am = aliases.get(aliases.size() - 1);
        assertEquals(am.getIdentity(), group.getExtension() + "@kuku");
        assertTrue(am.getContact().startsWith(group.getName() + "@kuku"));
    }

    private CallGroup createCallGroupWithUsers(String extension, String name, int numUsers) {
        CallGroup group = new CallGroup();
        group.setName(name);
        group.setExtension(extension);
        group.setVoicemailFallback(false);

        for (int i = 0; i < numUsers; i++) {
            User u = new User();
            u.setUserName("testUser" + i);
            group.insertRingForUser(u);
        }

        return group;
    }

    public void testGenerateAliasesNameAndExtSame() {
        CallGroup group = new CallGroup();
        group.setName("402");
        group.setExtension("402");
        group.setEnabled(true);
        List aliases = group.generateAliases("kiwi");
        assertEquals(0, aliases.size());
    }

    public void testClone() {
        CallGroup group = new CallGroup();
        group.setName("sales");
        group.setExtension("401");

        final int ringsLen = 5;
        for (int i = 0; i < ringsLen; i++) {
            User u = new User();
            u.setUserName("testUser" + i);
            group.insertRingForUser(u);
        }
        assertEquals(ringsLen, group.getRings().size());

        CallGroup clonedGroup = (CallGroup) group.duplicate();
        assertEquals("sales", clonedGroup.getName());
        assertEquals("401", clonedGroup.getExtension());
        List clonedCalls = clonedGroup.getRings();
        assertEquals(ringsLen, clonedCalls.size());
        for (int i = 0; i < ringsLen; i++) {
            UserRing ring = (UserRing) clonedCalls.get(i);
            assertEquals("testUser" + i, ring.getUser().getUserName());
            assertSame(clonedGroup, ring.getCallGroup());
        }
    }

    private double parseQ(String contact) {
        Scanner scanner = new Scanner(contact);
        scanner.findInLine("q=");
        if (scanner.hasNextDouble()) {
            return scanner.nextDouble();
        }
        return 0;
    }
}
