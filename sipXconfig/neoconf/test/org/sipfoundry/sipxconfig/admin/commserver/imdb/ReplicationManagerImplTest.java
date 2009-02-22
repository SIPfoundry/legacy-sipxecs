/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.apache.commons.codec.binary.Base64;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.device.InMemoryConfiguration;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;

import static org.easymock.EasyMock.aryEq;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class ReplicationManagerImplTest extends TestCase {

    private static final Location[] LOCATIONS = new Location[] {
        new Location(), new Location()
    };

    public void testReplicateFile() throws Exception {
        ReplicationManagerImpl replicationManager = new ReplicationManagerImpl();
        replicationManager.setHostname("localhost");

        final FileApi fileApi = createMock(FileApi.class);

        String content = "1234";
        fileApi.replace("localhost", "/etc/sipxecs/domain-config", 0644, encode(content));
        expectLastCall().andReturn(true).times(LOCATIONS.length);

        replay(fileApi);

        ApiProvider<FileApi> provider = new ApiProvider<FileApi>() {
            public FileApi getApi(String serviceUrl) {
                return fileApi;
            }
        };

        replicationManager.setFileApiProvider(provider);

        ConfigurationFile file = new InMemoryConfiguration("/etc/sipxecs", "domain-config", content);

        for (int i = 0; i < LOCATIONS.length; i++) {
            LOCATIONS[i].setRegistered(true);
        }
        replicationManager.replicateFile(LOCATIONS, file);

        verify(fileApi);
    }

    public void testReplicateData() {
        final Map<String, String> data[] = new Map[] {
            new HashMap<String, String>() {
            }
        };

        ReplicationManagerImpl replicationManager = new ReplicationManagerImpl();
        replicationManager.setHostname("localhost");

        final ImdbApi imdbApi = createMock(ImdbApi.class);

        imdbApi.replace(eq("localhost"), eq(DataSet.ALIAS.getName()), aryEq(data));
        expectLastCall().andReturn(true).times(LOCATIONS.length);
        replay(imdbApi);

        ApiProvider<ImdbApi> provider = new ApiProvider<ImdbApi>() {
            public ImdbApi getApi(String serviceUrl) {
                return imdbApi;
            }
        };

        replicationManager.setImdbApiProvider(provider);

        DataSetGenerator file = new DataSetGenerator() {

            protected void addItems(List<Map<String, String>> items) {
                items.add(data[0]);
            }

            protected DataSet getType() {
                return DataSet.ALIAS;
            }

        };

        replicationManager.replicateData(LOCATIONS, file);

        verify(imdbApi);
    }

    private String encode(String content) throws UnsupportedEncodingException {
        byte[] encoded = Base64.encodeBase64(content.getBytes("US-ASCII"));
        return new String(encoded, "US-ASCII");
    }
}
