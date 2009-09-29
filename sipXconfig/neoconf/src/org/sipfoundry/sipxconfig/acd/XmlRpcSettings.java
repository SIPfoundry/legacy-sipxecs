/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;

/**
 * Utility class that allows sending and retrieving settings through Provisioning interface.
 */
public class XmlRpcSettings {
    private static final Log LOG = LogFactory.getLog(XmlRpcSettings.class);

    private Provisioning m_provisioning;

    public XmlRpcSettings(Provisioning provisioning) {
        m_provisioning = provisioning;
    }

    boolean create(Setting setting) {
        Hashtable params = new Hashtable();
        XmlRpcParams xmlRpcParams = new XmlRpcParams(params, false);
        setting.acceptVisitor(xmlRpcParams);
        Map results = m_provisioning.create(params);
        boolean result = getResultCode(results);
        if (!result) {
            results = m_provisioning.set(params);
        }
        return getResultCode(results);
    }

    boolean delete(Setting setting) {
        Hashtable params = new Hashtable();
        setting.acceptVisitor(new XmlRpcParams(params, true));
        Map results = m_provisioning.delete(params);
        return getResultCode(results);
    }

    boolean set(Setting setting) {
        Hashtable params = new Hashtable();
        setting.acceptVisitor(new XmlRpcParams(params, false));
        Map results = m_provisioning.set(params);
        return getResultCode(results);
    }

    boolean get(Setting setting) {
        Hashtable params = new Hashtable();
        setting.acceptVisitor(new XmlRpcParams(params, false));
        Map results = m_provisioning.get(params);
        setting.acceptVisitor(new XmlRpcParamRetriever(results));
        return getResultCode(results);
    }

    Set getAll(String className, String key) {
        try {
            Set uris = new HashSet();
            Hashtable params = new Hashtable();
            setObjectClass(params, className);
            Map results = m_provisioning.get(params);
            boolean result = getResultCode(results);
            if (result) {
                List instances = getObjectClassList(results);
                for (Iterator i = instances.iterator(); i.hasNext();) {
                    Map objectData = (Map) i.next();
                    uris.add(objectData.get(key));
                }
            }
            return uris;
        } catch (XmlRpcRemoteException e) {
            // server throws exceptions if there are no objects of this class
            // it's should not be considered an error
            return Collections.EMPTY_SET;
        }
    }

    public boolean deleteAll(String objectClass, Collection uris, String key) {
        // do not bother making XML/RPC call if it's empty
        if (uris.isEmpty()) {
            return true;
        }
        boolean result = true;
        for (Iterator i = uris.iterator(); i.hasNext();) {
            Hashtable params = new Hashtable();
            setObjectClass(params, objectClass);
            params.put(key, i.next());
            Map results = m_provisioning.delete(params);
            result &= getResultCode(results);
        }
        return result;
    }

    private boolean getResultCode(Map results) {
        boolean resultCode = Provisioning.SUCCESS.equals(results.get("result-code"));
        if (!resultCode) {
            LOG.warn(results.get("result-text"));
        }
        return resultCode;
    }

    private static void setObjectClass(Map params, String objectClass) {
        params.put("object-class", objectClass);
    }

    private static List getObjectClassList(Map params) {
        return (List) params.get("object-class-list");
    }

    private static class XmlRpcParams extends AbstractSettingVisitor {
        private final Map m_parameters;
        private final boolean m_onlyRequired;

        public XmlRpcParams(Map parameters, boolean onlyRequired) {
            m_parameters = parameters;
            m_onlyRequired = onlyRequired;
        }

        public void visitSetting(Setting setting) {
            if (m_onlyRequired && !setting.getType().isRequired()) {
                return;
            }
            Object value = setting.getTypedValue();
            if (value != null) {
                // ignore null values - do not set them
                m_parameters.put(setting.getProfileName(), value);
            }
        }

        public boolean visitSettingGroup(SettingSet group) {
            setObjectClass(m_parameters, group.getProfileName());
            return true;
        }
    }

    private static class XmlRpcParamRetriever extends AbstractSettingVisitor {
        private final Map m_parameters;

        public XmlRpcParamRetriever(Map parameters) {
            m_parameters = parameters;
        }

        public void visitSetting(Setting setting) {
            String profileName = setting.getProfileName();
            Object value = m_parameters.get(profileName);
            if (value != null) {
                setting.setValue(value.toString());
            }
        }
    }
}
