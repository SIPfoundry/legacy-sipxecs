/**
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.util;


/**
 * @author Mardy Marshall
 *
 */
public class IPAddressUtil {
	/**
	 * Test to see if the given address string represents a literal IPv4 address.
	 *
	 * @param address
	 *           The address string to be tested.
	 *
	 * @return
	 *           True if literal IPv4 address, False otherwise.
	 */
	public static boolean isLiteralIPAddress(String address) {
		String[] octets= address.split("\\.", -1);
		if (octets.length != 4) {
			return false;
		}

		for (int i = 0; i < 4; i++) {
			int octetValue = -1;
			try {
				octetValue = Integer.parseInt(octets[i]);
			} catch (NumberFormatException e) {
				return false;
			}
			if (octetValue < 0 || octetValue > 0xff) {
				return false;
			}
		}

		// All checks passed.
		return true;
	}

}
