/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import static java.util.Arrays.asList;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.Model;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.service.SipxServiceBundle.TooFewBundles;
import org.sipfoundry.sipxconfig.service.SipxServiceBundle.TooManyBundles;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxServiceManagerImplTest extends TestCase {
    public void testVerifyBundleCardinality() {
        SipxServiceBundle b_0_0 = new SipxServiceBundle("0-0");
        SipxServiceBundle b_0_1 = new SipxServiceBundle("0-1");
        b_0_1.setMax(1);

        SipxServiceBundle b_1_1 = new SipxServiceBundle("1-1");
        b_1_1.setMin(1);
        b_1_1.setMax(1);

        SipxServiceBundle b_1_5 = new SipxServiceBundle("1-5");
        b_1_5.setMin(1);
        b_1_5.setMax(5);

        Location l1 = new Location();
        l1.setUniqueId();
        l1.setInstalledBundles(asList("0-0", "0-1"));

        Location l2 = new Location();
        l2.setUniqueId();
        l2.setInstalledBundles(asList("0-0", "1-1"));

        Location l3 = new Location();
        l3.setUniqueId();
        l3.setInstalledBundles(asList("0-0", "1-5"));

        LocationsManager lm = createMock(LocationsManager.class);
        lm.getLocations();
        expectLastCall().andReturn(new Location[] {
            l1, l2, l3
        }).anyTimes();

        replay(lm);

        BundleModelSource modelSource = new BundleModelSource(b_0_0, b_0_1, b_1_1, b_1_5);
        SipxServiceManagerImpl sm = new SipxServiceManagerImpl();
        sm.setLocationsManager(lm);
        sm.setBundleModelSource(modelSource);

        try {
            sm.verifyBundleCardinality(l1, Collections.<SipxServiceBundle> emptyList());
            sm.verifyBundleCardinality(l2, asList(b_1_1));
            sm.verifyBundleCardinality(l3, asList(b_1_5));
        } catch (UserException e) {
            fail("Unexpected exception: " + e);
        }

        try {
            sm.verifyBundleCardinality(l1, asList(b_1_1, b_0_0));
            fail("b_1_1 is now at 2 locations - need an exception here");
        } catch (TooManyBundles e) {
            // ok
        }

        try {
            sm.verifyBundleCardinality(l3, asList(b_0_0));
            fail("b_1_5 is not acitve anywhere - need an exception here");
        } catch (TooFewBundles e) {
            // ok
        }

        verify(lm);
    }

    public void testLocationResetBundles() {
        Location l1 = new Location();
        l1.setUniqueId();
        l1.setInstalledBundles(asList("b"));

        SipxServiceBundle b = new SipxServiceBundle("b");

        SipxService service1 = new SipxAcdService();
        service1.setProcessName("acd");
        service1.setBundles(Collections.<SipxServiceBundle> emptySet());

        SipxServiceManagerImpl sm = new SipxServiceManagerImpl() {
            @Override
            Collection<SipxService> getServicesFromDb() {
                return Collections.emptyList();
            }
        };

        BundleModelSource bms = new BundleModelSource(b);
        sm.setBundleModelSource(bms);
        ServiceModelSource sms = new ServiceModelSource(service1);
        sm.setServiceModelSource(sms);

        LocationsManager lm = createMock(LocationsManager.class);
        lm.getLocations();
        expectLastCall().andReturn(new Location[] {
            l1
        }).anyTimes();
        replay(lm);
        sm.setLocationsManager(lm);

        l1.resetBundles(sm);
        Collection<SipxService> sipxServices = l1.getSipxServices();
        assertTrue("should be empty - no services in the bundle", sipxServices.isEmpty());

        // add service to the bundle and reset the bundles
        service1.setBundles(Collections.singleton(b));
        l1.resetBundles(sm);

        sipxServices = l1.getSipxServices();
        assertFalse(sipxServices.isEmpty());
        assertSame(service1, sipxServices.iterator().next());

        verify(lm);
    }

    public void testGetServiceParam() {
        SipxServiceManagerImpl sm = new SipxServiceManagerImpl() {
            @Override
            Collection<SipxService> getServicesFromDb() {
                return Collections.emptyList();
            }

            @Override
            public boolean isServiceInstalled(String serviceBeanId) {
                return true;
            }
        };

        SipxService service1 = new SipxProxyService();
        service1.setBeanId(SipxProxyService.BEAN_ID);

        SipxService service2 = new SipxRegistrarService() {
            @Override
            public Object getParam(String paramName) {
                if (paramName.equals("pp")) {
                    return "qq";
                }
                return super.getParam(paramName);
            }
        };

        service2.setBeanId(SipxRegistrarService.BEAN_ID);

        ModelSource<SipxService> modelSource = new ServiceModelSource(service1, service2);
        sm.setServiceModelSource(modelSource);

        assertNull(sm.getServiceParam("bongo"));
        assertEquals("qq", sm.getServiceParam("pp"));
    }

    public void testGetServiceParamNotInstalled() {
        SipxServiceManagerImpl sm = new SipxServiceManagerImpl() {
            @Override
            Collection<SipxService> getServicesFromDb() {
                return Collections.emptyList();
            }

            @Override
            public boolean isServiceInstalled(String serviceBeanId) {
                return false;
            }
        };

        SipxService service1 = new SipxRegistrarService() {
            @Override
            public Object getParam(String paramName) {
                if (paramName.equals("pp")) {
                    return "qq";
                }
                return super.getParam(paramName);
            }
        };
        service1.setBeanId(SipxRegistrarService.BEAN_ID);

        ModelSource<SipxService> modelSource = new ServiceModelSource(service1);
        sm.setServiceModelSource(modelSource);

        assertNull(sm.getServiceParam("pp"));
    }

    abstract static class SimpleModelSource<T extends Model> implements ModelSource<T> {
        Map<String, T> m_map = new HashMap<String, T>();

        public SimpleModelSource(T... bundles) {
            for (T bundle : bundles) {
                String modelId = getModelId(bundle);
                bundle.setBeanName(modelId);
                m_map.put(modelId, bundle);
            }
        }

        public T getModel(String modelId) {
            T bundle = m_map.get(modelId);
            if (bundle == null) {
                fail("Do not know this modelId: [" + modelId + "]");
            }
            return bundle;
        }

        public Collection<T> getModels() {
            return m_map.values();
        }

        protected abstract String getModelId(T model);
    }

    private static class BundleModelSource extends SimpleModelSource<SipxServiceBundle> {
        public BundleModelSource(SipxServiceBundle... bundles) {
            super(bundles);
        }

        @Override
        protected String getModelId(SipxServiceBundle model) {
            return model.getName();
        }
    }

    private static class ServiceModelSource extends SimpleModelSource<SipxService> {
        public ServiceModelSource(SipxService... services) {
            super(services);
        }

        @Override
        protected String getModelId(SipxService model) {
            return model.getProcessName();
        }
    }
}
