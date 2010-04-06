/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.alarm;

import java.net.URL;

import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;

public class SipXAlarmClient {
	private XmlRpcClient xmlRpcClient;
	private static final String protocol = "https";
	private String serverDomain;
	private int port;

	private static final String METHOD_NAME = "Alarm.raiseAlarm";

	private void init() {
		try {

			XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();

			config.setServerURL(new URL(protocol + "://" + serverDomain + ":"
					+ port + "/RPC2"));
			config.setEnabledForExceptions(true);
			config.setEnabledForExtensions(true);
			config.setConnectionTimeout(1);
			this.xmlRpcClient = new XmlRpcClient();
			xmlRpcClient.setConfig(config);
			xmlRpcClient.setMaxThreads(1);

		} catch (Exception ex) {
			throw new RuntimeException("unexpected error", ex);
		}
	}

	public SipXAlarmClient(String host, int port)
			throws IllegalArgumentException {
		try {

			this.serverDomain = host;
			this.port = port;
		} catch (Exception ex) {
			throw new IllegalArgumentException("IllegalArgument " + host + ":"
					+ port);
		}

	}

	public void raiseAlarm(String alarmId, String... alarmParam)
			throws XmlRpcException {

		if (this.xmlRpcClient == null) {
			init();
		}

		Object[] args = new Object[3];
		args[0] = this.serverDomain;
		args[1] = alarmId;
		String[] alarmBody = null;

		if (alarmParam != null) {
			alarmBody = new String[alarmParam.length];
			int k = 0;
			for (String alarm : alarmParam) {
				alarmBody[k++] = alarm;
			}
		}
		args[2] = alarmBody;
		this.xmlRpcClient.execute(METHOD_NAME, args);

	}

}
