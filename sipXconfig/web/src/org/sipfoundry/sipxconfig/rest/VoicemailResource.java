/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.rest;

import static org.apache.commons.lang.StringUtils.join;
import static org.restlet.data.MediaType.APPLICATION_RSS_XML;

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.OutputRepresentation;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.login.PrivateUserKeyManager;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.Voicemail;
import org.springframework.beans.factory.annotation.Required;

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

public class VoicemailResource extends UserResource {
    private static final String WAV = "wav";
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

        List<Voicemail> voicemails = m_mailboxManager.getVoicemail(getUser().getUserName(), m_folder);
        List<SyndEntry> entries = new ArrayList<SyndEntry>(voicemails.size());
        for (Voicemail voicemail : voicemails) {
            String messageUrl = joinUrl(m_folderUrl, voicemail.getMessageId());
            SyndEntry entry = new SyndEntryImpl();
            entry.setTitle(String.format("%s from %s", voicemail.getSubject(), voicemail.getFromBrief()));
            entry.setPublishedDate(voicemail.getTimestamp());
            entry.setLink(messageUrl);

            SyndContent content = new SyndContentImpl();
            content.setValue(String.format("%s seconds received at %2$tR on %2$tD",
                    voicemail.getDurationsecs(), voicemail.getTimestamp()));
            entry.setDescription(content);

            SyndEnclosureImpl enclosure = new SyndEnclosureImpl();
            enclosure.setType(StringUtils.equals(voicemail.getAudioFormat(), WAV) ? "audio/wav" : "audio/mpeg");
            enclosure.setUrl(messageUrl);
            entry.setEnclosures(Collections.singletonList(enclosure));

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
