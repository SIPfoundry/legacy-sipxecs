/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.rest;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.acegisecurity.Authentication;
import org.acegisecurity.context.SecurityContextHolder;
import org.apache.commons.io.IOUtils;
import org.apache.commons.io.output.ByteArrayOutputStream;
import org.restlet.data.MediaType;
import org.restlet.data.Reference;
import org.restlet.data.Request;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.Voicemail;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.replay;

public class VoicemailResourceDownloadTest extends TestCase {

    private VoicemailResourceDownload m_resource;

    @Override
    protected void setUp() throws Exception {
        User user = new User();
        user.setUniqueId();
        user.setUserName("joeuser");

        Authentication token = new TestAuthenticationToken(user, false, false).authenticateToken();
        SecurityContextHolder.getContext().setAuthentication(token);

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUser(user.getId());
        expectLastCall().andReturn(user);

        Mailbox mailbox = new Mailbox(null, "joeuser");
        File vmailDir = new File("");
        Voicemail vmail = new Voicemail(vmailDir, "joeuser", "inbox", "007") {
            @Override
            public File getMediaFile() {
                return new File(TestUtil.getTestSourceDirectory(getClass()), "007.wav");
            }
        };

        List<Voicemail> vmails = new ArrayList<Voicemail>();
        vmails.add(vmail);

        MailboxManager mailboxManager = createMock(MailboxManager.class);
        mailboxManager.getMailbox(user.getUserName());
        expectLastCall().andReturn(mailbox);
        mailboxManager.getVoicemail(mailbox, "inbox");
        expectLastCall().andReturn(vmails);
        mailboxManager.markRead(mailbox, vmail);
        expectLastCall();
        replay(coreContext, mailboxManager);

        m_resource = new VoicemailResourceDownload();
        m_resource.setMailboxManager(mailboxManager);
        m_resource.setCoreContext(coreContext);
    }

    public void testDownload() throws Exception {
        runTest("007");
    }

    public void testDownloadWithWavSuffix() throws Exception {
        runTest("007.wav");
    }

    public void runTest(String messageId)  throws Exception {
        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("folder", "inbox");
        attributes.put("messageId", messageId);
        request.setAttributes(attributes);
        request.setOriginalRef(new Reference("https://example.com:8034/sipxconfig/rest/my/voicemail/inbox/007"));
        request.setRootRef(new Reference("https://example.com:8034/sipxconfig/rest"));

        m_resource.init(null, request, null);

        Representation representation = m_resource.represent(new Variant(MediaType.AUDIO_WAV));

        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        representation.write(outputStream);

        byte[] outputBytes = outputStream.toByteArray();
        byte[] expectedBytes = IOUtils.toByteArray(getClass().getResourceAsStream("007.wav"));

        assertTrue(Arrays.equals(expectedBytes, outputBytes));
    }
}
