/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.phone.polycom;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class PhoneUpdateResourceTest extends TestCase {
    private PhoneContext m_phoneContext;
    private ProfileManager m_profileManager;
    private PhoneUpdateResource m_resource = new PhoneUpdateResource();
    PolycomPhone m_polycom300 = new PolycomPhone();
    PolycomPhone m_polycom301 = new PolycomPhone();
    PolycomPhone m_polycom335 = new PolycomPhone();
    

    
    @Override
    protected void setUp() throws Exception {
        
        m_polycom300.setBeanId("polycom");
        m_polycom300.setModel(PolycomXmlTestCase.phoneModelBuilder("polycom300", getClass()));
        m_polycom300.setModelId("polycom300");
        m_polycom300.setDeviceVersion(PolycomModel.VER_2_0);
        m_polycom300.setUniqueId(1);

        
        m_polycom301.setBeanId("polycom");
        m_polycom301.setModel(PolycomXmlTestCase.phoneModelBuilder("polycom300", getClass()));
        m_polycom301.setModelId("polycom300");
        m_polycom301.setDeviceVersion(PolycomModel.VER_2_0);
        m_polycom301.setUniqueId(2);
        
        
        m_polycom335.setBeanId("polycom");
        m_polycom335.setModel(PolycomXmlTestCase.phoneModelBuilder("polycom335", getClass()));
        m_polycom335.setModelId("polycom335");
        m_polycom335.setDeviceVersion(PolycomModel.VER_4_0_X);
        m_polycom335.setUniqueId(3);
        
        m_phoneContext = EasyMock.createMock(PhoneContext.class);
        m_profileManager = EasyMock.createMock(ProfileManager.class);
    }
    
    /*
     * no longer there are unsupported poly models 
     *
     */
    public void _testDeleteUnsupported() throws Exception {
        m_phoneContext.getPhoneIdBySerialNumber("111111111111");
        expectLastCall().andReturn(1);
        m_phoneContext.loadPhone(1);
        expectLastCall().andReturn(m_polycom300);
        m_phoneContext.deletePhone(m_polycom300);
        expectLastCall();
        
        replay(m_phoneContext);

        m_resource.setPhoneContext(m_phoneContext);
        
        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("mac", "111111111111");
        attributes.put("version", "2.0.X");
        attributes.put("model", "polycom300");
        request.setAttributes(attributes);
        
        m_resource.setRequest(request);
        m_resource.init(null, request, new Response(request));
        
        m_resource.represent(new Variant(MediaType.ALL));
        verify(m_phoneContext);
    }
    
    public void testUpdateFwAndModel301() throws Exception {
        m_phoneContext.getPhoneIdBySerialNumber("111111111112");
        expectLastCall().andReturn(2);
        m_phoneContext.loadPhone(2);
        expectLastCall().andReturn(m_polycom301);
        m_phoneContext.storePhone(m_polycom301);
        expectLastCall();

        replay(m_phoneContext);

        m_resource.setPhoneContext(m_phoneContext);
        m_resource.setProfileManager(m_profileManager);
      
        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("mac", "111111111112");
        attributes.put("version", "3.1.X");
        attributes.put("model", "polycom301");
        request.setAttributes(attributes);
        
        m_resource.setRequest(request);
        m_resource.init(null, request, new Response(request));
        
        m_resource.represent(new Variant(MediaType.ALL));
        verify(m_phoneContext);
        m_polycom301.getModelId().equals("polycom301");
        m_polycom301.getDeviceVersion().equals(PolycomModel.VER_3_1_X);
    }

    public void testNotUpdateFw() throws Exception {
        m_phoneContext.getPhoneIdBySerialNumber("111111111113");
        expectLastCall().andReturn(3);
        m_phoneContext.loadPhone(3);
        expectLastCall().andReturn(m_polycom335);

        replay(m_phoneContext);

        m_resource.setPhoneContext(m_phoneContext);
        m_resource.setProfileManager(m_profileManager);
      
        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("mac", "111111111113");
        attributes.put("version", "4.0.X");
        attributes.put("model", "polycom335");
        request.setAttributes(attributes);
        
        m_resource.setRequest(request);
        m_resource.init(null, request, new Response(request));
        
        m_resource.represent(new Variant(MediaType.ALL));
        verify(m_phoneContext);
        m_polycom335.getModelId().equals("polycom335");
        m_polycom335.getDeviceVersion().equals(PolycomModel.VER_4_0_X);
    }
    
}
