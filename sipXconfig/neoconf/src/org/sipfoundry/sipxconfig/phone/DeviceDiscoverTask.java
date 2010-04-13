/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;
import org.sipfoundry.sipxconfig.device.DiscoveredDevice;

public class DeviceDiscoverTask implements Runnable {

    static final String PREFLIGHT_BINARY = "preflight";

    private final DeviceFinder m_deviceFinder;

    /**
     * @param deviceFinder
     */
    DeviceDiscoverTask(DeviceFinder deviceFinder) {
        m_deviceFinder = deviceFinder;
    }

    Document getDiscoveredDevicesXmlDocument() {
        Document document = null;
        File executable = new File(m_deviceFinder.getBinDirectory(), PREFLIGHT_BINARY);
        List<String> cmds = new ArrayList<String>();
        cmds.add(executable.getAbsolutePath());
        cmds.add("--discover");
        String[] cmd = cmds.toArray(new String[cmds.size()]);
        try {
            Process process = Runtime.getRuntime().exec(cmd);
            process.waitFor();
            SAXReader reader = new SAXReader();
            document = reader.read(process.getInputStream());
        } catch (IOException io) {
            DeviceFinder.LOG.error("Preflight is not installed");
        } catch (InterruptedException ie) {
            DeviceFinder.LOG.error("Errors when executing preflight:" + ie);
        } catch (DocumentException dex) {
            DeviceFinder.LOG.error("Preflight returns invalid document");
        }
        return document;
    }

    boolean discover() {
        List<DiscoveredDevice> devices = new ArrayList<DiscoveredDevice>();
        Document document = getDiscoveredDevicesXmlDocument();
        if (document != null) {
            Element root = document.getRootElement();
            for (Iterator i = root.elementIterator("device"); i.hasNext();) {
                Element deviceXml = (Element) i.next();
                DiscoveredDevice device = new DiscoveredDevice();
                String macAddress = deviceXml.element("hardware-address").getText();
                macAddress = macAddress.replace(":", "");
                device.setMacAddress(macAddress.toLowerCase());
                device.setIpAddress(deviceXml.element("network-address").getText());
                device.setVendor(deviceXml.element("vendor").getText());
                devices.add(device);
            }
            m_deviceFinder.setDevices(devices);
            return true;
        }
        return false;
    }

    public void run() {
        m_deviceFinder.setState(DeviceFinder.RUNNING);
        if (discover()) {
            this.m_deviceFinder.setState(DeviceFinder.FINISHED);
        } else {
            this.m_deviceFinder.setState(DeviceFinder.FAILED);
        }
    }
}
