/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.alarm;

import org.apache.xmlrpc.XmlRpcException;

public class SipXAlarmClient {

	public SipXAlarmClient() {
	    // TODO: send something to local snmpd process
	}

	@Deprecated
	public SipXAlarmClient(String host, int port)
			throws IllegalArgumentException {
	}

	public void raiseAlarm(String alarmId, String... alarmParam)
			throws XmlRpcException {

//		if (this.xmlRpcClient == null) {
//			init();
//		}
//
//		Object[] args = new Object[3];
//		args[0] = this.serverDomain;
//		args[1] = alarmId;
//		String[] alarmBody = null;
//
//		if (alarmParam != null) {
//			alarmBody = new String[alarmParam.length];
//			int k = 0;
//			for (String alarm : alarmParam) {
//				alarmBody[k++] = alarm;
//			}
//		}
//		args[2] = alarmBody;
//		this.xmlRpcClient.execute(METHOD_NAME, args);

	}

}
