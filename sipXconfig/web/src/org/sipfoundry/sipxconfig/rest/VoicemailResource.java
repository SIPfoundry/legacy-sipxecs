/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.rest;

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;

import com.sun.syndication.feed.synd.SyndContent;
import com.sun.syndication.feed.synd.SyndContentImpl;
import com.sun.syndication.feed.synd.SyndEnclosureImpl;
import com.sun.syndication.feed.synd.SyndEntry;
import com.sun.syndication.feed.synd.SyndEntryImpl;
import com.sun.syndication.feed.synd.SyndFeed;
import com.sun.syndication.feed.synd.SyndFeedImpl;
import com.sun.syndication.io.FeedException;
import com.sun.syndication.io.SyndFeedOutput;
import edu.emory.mathcs.backport.java.util.Collections;
import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.OutputRepresentation;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.login.PrivateUserKeyManager;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.Voicemail;
import org.sipfoundry.sipxconfig.vm.Voicemail.MessageDescriptor;
import org.springframework.beans.factory.annotation.Required;

import static org.apache.commons.lang.StringUtils.join;
import static org.restlet.data.MediaType.APPLICATION_RSS_XML;

public class VoicemailResource extends UserResource {
    private static final String UTF_8 = "UTF-8";

    private String m_folder;
    private String m_url;
    private String m_folderUrl;

    private MailboxManager m_mailboxManager;

    private PrivateUserKeyManager m_privateUserKeyManager;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        m_folder = (String) getRequest().getAttributes().get("folder");
        m_url = request.getOriginalRef().getIdentifier();
        String rootUrl = request.getRootRef().getIdentifier();
        String privateKeyForUser = m_privateUserKeyManager.getPrivateKeyForUser(getUser());
        // URL /private/{securekey}/voicemail/{folder}/{messageId}
        m_folderUrl = String.format("%s/private/%s/voicemail/%s", rootUrl, privateKeyForUser, m_folder);
        getVariants().add(new Variant(APPLICATION_RSS_XML));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        SyndFeed feed = new SyndFeedImpl();
        feed.setFeedType("rss_2.0");
        feed.setEncoding(UTF_8);
        feed.setTitle("Voicemail: " + getUser().getLabel());
        feed.setLink(m_url);
        feed.setDescription("Voicemail Inbox for " + getUser().getLabel());

        Mailbox mailbox = m_mailboxManager.getMailbox(getUser().getUserName());
        List<Voicemail> voicemails = m_mailboxManager.getVoicemail(mailbox, m_folder);
        List<SyndEntry> entries = new ArrayList<SyndEntry>(voicemails.size());
        for (Voicemail voicemail : voicemails) {
            MessageDescriptor md = voicemail.getDescriptor();
            String messageUrl = joinUrl(m_folderUrl, voicemail.getMessageId());
            SyndEntry entry = new SyndEntryImpl();
            entry.setTitle(String.format("%s from %s", md.getSubject(), md.getFromBrief()));
            entry.setPublishedDate(md.getTimestamp());
            entry.setLink(messageUrl);

            SyndContent content = new SyndContentImpl();
            content.setValue(String.format("%s seconds received at %2$tR on %2$tD", md.getDurationsecs(), md
                    .getTimestamp()));
            entry.setDescription(content);

            SyndEnclosureImpl wav = new SyndEnclosureImpl();
            wav.setType("audio/x-wav");
            wav.setUrl(messageUrl);
            entry.setEnclosures(Collections.singletonList(wav));

            entries.add(entry);
        }

        feed.setEntries(entries);
        return new SyndRepresentation(feed);
    }

    private class SyndRepresentation extends OutputRepresentation {
        private final SyndFeed m_feed;

        public SyndRepresentation(SyndFeed feed) {
            super(APPLICATION_RSS_XML);
            m_feed = feed;
        }

        @Override
        public void write(OutputStream outputStream) throws IOException {
            try {
                Writer writer = new OutputStreamWriter(outputStream, UTF_8);
                SyndFeedOutput feedOutput = new SyndFeedOutput();
                feedOutput.output(m_feed, writer);
            } catch (FeedException e) {
                new RuntimeException(e);
            }
        }
    }

    @Required
    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

    @Required
    public void setPrivateUserKeyManager(PrivateUserKeyManager privateUserKeyManager) {
        m_privateUserKeyManager = privateUserKeyManager;
    }

    private static String joinUrl(String... items) {
        return join(items, "/");
    }
}
