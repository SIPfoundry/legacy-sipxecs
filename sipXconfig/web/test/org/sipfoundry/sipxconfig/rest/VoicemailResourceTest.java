/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rest;

import java.io.File;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TimeZone;

import org.sipfoundry.sipxconfig.login.PrivateUserKeyManager;

import junit.framework.TestCase;
import org.acegisecurity.Authentication;
import org.acegisecurity.context.SecurityContextHolder;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.restlet.data.Reference;
import org.restlet.data.Request;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.Voicemail;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.restlet.data.MediaType.APPLICATION_RSS_XML;

public class VoicemailResourceTest extends TestCase {
    private TimeZone m_timeZone;

    @Override
    protected void setUp() throws Exception {
        m_timeZone = TimeZone.getDefault();
        TimeZone.setDefault(TimeZone.getTimeZone("GMT"));
    }

    @Override
    protected void tearDown() throws Exception {
        TimeZone.setDefault(m_timeZone);
        SecurityContextHolder.getContext().setAuthentication(null);
    }

    public void testFeed() throws Exception {
        User user = new User();
        user.setUniqueId();
        user.setUserName("joeuser");

        Authentication token = new TestAuthenticationToken(user, false, false).authenticateToken();
        SecurityContextHolder.getContext().setAuthentication(token);

        Mailbox mailbox = new Mailbox(null, "joeuser");

        List<Voicemail> vmails = new ArrayList<Voicemail>();
        File vmailDir = new File("/voicemail/store");
        for (int i = 0; i < 5; i++) {
            final int id = i;
            Voicemail vmail = new Voicemail(vmailDir, "joeuse", "inbox", "0000000" + i) {
                @Override
                protected MessageDescriptor readMessageDescriptor(File file) {
                    Calendar cal = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
                    cal.set(1970, Calendar.JANUARY, 14 + id, 10, 0, id);

                    MessageDescriptor md = createMock(MessageDescriptor.class);
                    md.getSubject();
                    expectLastCall().andReturn("Voice Message 0000000" + id);
                    md.getTimestamp();
                    expectLastCall().andReturn(cal.getTime()).atLeastOnce();
                    md.getFrom();
                    expectLastCall().andReturn("George <president@example.org>");
                    md.getFromBrief();
                    expectLastCall().andReturn("George");
                    md.getDurationsecs();
                    expectLastCall().andReturn(10 + id);
                    replay(md);

                    return md;
                }
            };
            vmails.add(vmail);
        }

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUser(user.getId());
        expectLastCall().andReturn(user);

        MailboxManager mailboxManager = createMock(MailboxManager.class);
        mailboxManager.getMailbox(user.getUserName());
        expectLastCall().andReturn(mailbox);
        mailboxManager.getVoicemail(mailbox, "inbox");
        expectLastCall().andReturn(vmails);

        PrivateUserKeyManager pukm = createMock(PrivateUserKeyManager.class);
        pukm.getPrivateKeyForUser(user);
        expectLastCall().andStubReturn("123456");

        replay(coreContext, mailboxManager, pukm);

        VoicemailResource resource = new VoicemailResource();

        resource.setMailboxManager(mailboxManager);
        resource.setCoreContext(coreContext);
        resource.setPrivateUserKeyManager(pukm);

        Request request = new Request();
        Map<String, Object> attributes = new HashMap();
        attributes.put("folder", "inbox");
        request.setAttributes(attributes);
        request.setOriginalRef(new Reference("https://example.com:8034/sipxconfig/rest/my/feed/voicemail/inbox"));
        request.setRootRef(new Reference("https://example.com:8034/sipxconfig/rest"));

        resource.init(null, request, null);

        Representation representation = resource.represent(new Variant(APPLICATION_RSS_XML));

        StringWriter writer = new StringWriter();
        representation.write(writer);

        String generated = StringUtils.trim(StringUtils.replace(writer.toString(), "\r\n", "\n"));
        // FileUtils.writeStringToFile(new File("kuku"), generated);
        String expected = IOUtils.toString(getClass().getResourceAsStream("voicemail-feed.test.xml"));

        assertEquals(expected, generated);
    }
}
