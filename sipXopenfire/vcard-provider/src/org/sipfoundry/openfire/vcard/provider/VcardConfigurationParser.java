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
package org.sipfoundry.openfire.vcard.provider;

import java.io.IOException;

import org.apache.commons.digester.Digester;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

public class VcardConfigurationParser {
	public static final String CONFIG = "sipxopenfire-config";


    private static String currentTag = null;
    private static Digester digester;

    static {
        Logger logger = Logger.getLogger(Digester.class);
        logger.addAppender(new ConsoleAppender(new SimpleLayout()));
        logger.setLevel(Level.OFF);
        logger = Logger.getLogger("org.apache.commons.beanutils");
        logger.addAppender(new ConsoleAppender(new SimpleLayout()));
        logger.setLevel(Level.OFF);
    }

	private static void addCallMethodInt(String elementName, String methodName) {
		digester.addCallMethod(String.format("%s/%s", currentTag, elementName),
				methodName, 0, new Class[] { Integer.class });
	}

    /*
    * Add the digester rules.
    *
    * @param digester
    */
   private static void addRules(Digester digester) {
	   digester.setUseContextClassLoader(true);
       digester.addObjectCreate(CONFIG, VcardConfig.class.getName());

       currentTag = CONFIG;
       addCallMethodInt("openfire-xml-rpc-vcard-port", "setOpenfireXmlRpcVcardPort");
    }

   public VcardConfig parse(String url) throws SAXException, IOException {
       digester = new Digester();
       addRules(digester);
       InputSource inputSource = new InputSource(url);
       digester.parse(inputSource);
       VcardConfig vcardConfig = (VcardConfig) digester.getRoot();
       return vcardConfig;       
   }
}
