package org.sipfoundry.openfire.config;

public class XmppTransportRegistration implements Comparable<XmppTransportRegistration> {

	private String user = "";
	private String transportType = "";
	private String legacyUsername = "";
	private String legacyPassword = "";
	private String legacyNickname = "";

	public XmppTransportRegistration() {
	}

	public String getUser() {
		return this.user;
	}

	public void setUser(String user) {
		this.user = user;
	}

	public String getTransportType() {
		return this.transportType;
	}

	public void setTransportType(String transportType) {
		this.transportType = transportType;
	}

	public String getLegacyUsername() {
		return this.legacyUsername;
	}

	public void setLegacyUsername(String legacyUsername) {
		this.legacyUsername = legacyUsername;
	}

	public String getLegacyPassword() {
		return this.legacyPassword;
	}

	public void setLegacyPassword(String legacyPassword) {
		this.legacyPassword = legacyPassword;
	}

	public String getLegacyNickname() {
		return this.legacyNickname;
	}

	public void setLegacyNickname(String legacyNickname) {
		this.legacyNickname = legacyNickname;
	}

	public int compareTo(XmppTransportRegistration registration) {

		int result;

		// The first key is transport.
		result = this.transportType.compareTo(registration.getTransportType());
		if (result != 0) {
			return result;
		}

		// The second key is user.
		result = this.user.compareTo(registration.getUser());
		if (result != 0) {
			return result;
		}

		// The third key is legacy user name.
		result = this.legacyUsername.compareTo(registration.getLegacyUsername());
		if (result != 0) {
			return result;
		}

		// The fourth key is legacy password.
		result = this.legacyPassword.compareTo(registration.getLegacyPassword());
		if (result != 0) {
			return result;
		}

		// The fifth key is legacy nickname.
		result = this.legacyNickname.compareTo(registration.getLegacyNickname());
		if (result != 0) {
			return result;
		}

		return result;
	}

	@Override
	public boolean equals(Object obj) {

		if (this == obj) {
			return true;
		}

		if (!(obj instanceof XmppTransportRegistration)) {
			return false;
		}

		XmppTransportRegistration registration = (XmppTransportRegistration) obj;

		Boolean result = (this.transportType.equals(registration.getTransportType())
				&& this.user.equals(registration.getUser())
				&& this.legacyUsername.equals(registration.getLegacyUsername())
				&& this.legacyPassword.equals(registration.getLegacyPassword()) && this.legacyNickname
				.equals(registration.getLegacyNickname()));

		return result;
	}

	@Override
	public int hashCode() {

		int HASH_PRIME = 1000003;
		int result = 0;

		result = HASH_PRIME * result + this.transportType.hashCode();
		result = HASH_PRIME * result + this.user.hashCode();
		result = HASH_PRIME * result + this.legacyUsername.hashCode();
		result = HASH_PRIME * result + this.legacyPassword.hashCode();
		result = HASH_PRIME * result + this.legacyNickname.hashCode();

		return result;
	}

	public String toString() {

		return new StringBuilder(" user=").append(this.getUser()).append(" transport=").append(
				this.getTransportType()).append(" legacy_user=").append(this.getLegacyUsername())
				.append(" legacy_password=").append(this.getLegacyPassword()).append(
						" legacy_nickname=").append(this.getLegacyNickname()).toString();
	}
}
