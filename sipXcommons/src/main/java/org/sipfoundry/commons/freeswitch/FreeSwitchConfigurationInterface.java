package org.sipfoundry.commons.freeswitch;

import org.apache.log4j.Logger;

public interface FreeSwitchConfigurationInterface {

	public abstract Logger getLogger();

	public abstract String getLogLevel();

	public abstract String getLogFile();

	public abstract int getEventSocketPort();

    public abstract String getDocDirectory();

	public abstract String getSipxchangeDomainName();

	public abstract String getRealm();

	public abstract void setRealm(String realm);

}
