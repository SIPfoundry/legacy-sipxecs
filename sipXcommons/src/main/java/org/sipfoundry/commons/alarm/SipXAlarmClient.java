package org.sipfoundry.commons.alarm;

import java.net.URL;

import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import org.sipfoundry.commons.util.Hostname;

public class SipXAlarmClient {
	private XmlRpcClient xmlRpcClient;
	private static final String protocol = "https";
	private String serverDomain;
	
	
	private static final String METHOD_NAME = "Alarm.raiseAlarm";
	
	public SipXAlarmClient(String host, int port)
			throws IllegalArgumentException {
		try {
			serverDomain = host;
			XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();

			config.setServerURL(new URL(protocol + "://" + serverDomain + ":"
					+ port + "/RPC2"));
			config.setEnabledForExceptions(true);
			config.setEnabledForExtensions(true);
			this.xmlRpcClient = new XmlRpcClient();
			xmlRpcClient.setConfig(config);
			xmlRpcClient.setMaxThreads(32);

		} catch (Exception ex) {
			throw new IllegalArgumentException("IllegalArgument " + host + ":"
					+ port);
		}

	}
	
	public  void raiseAlarm(String alarmId, String... alarmParam) throws XmlRpcException {
	   Object[] args  = new Object[3];
	   args[0] = this.serverDomain;
	   args[1] = alarmId;
	   String[] alarmBody  = null ;
	   
	  if ( alarmParam != null ) {
		  alarmBody = new String[alarmParam.length];
		  int k = 0;
		  for ( String alarm : alarmParam) {
			  alarmBody[k++] = alarm;
		  }
	  }
	  args[2] = alarmBody;
	   
	   
	   this.xmlRpcClient.execute(METHOD_NAME,args);
	}

}
