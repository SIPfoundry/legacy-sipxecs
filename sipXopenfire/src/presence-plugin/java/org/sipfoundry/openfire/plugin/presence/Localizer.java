/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.plugin.presence;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;
import org.apache.log4j.Logger;

public class Localizer
{
    private static Logger log = Logger.getLogger(Localizer.class);

    Properties localizationTable;
    
    Localizer( Properties props ){
        localizationTable = props;
    }

    String localize( String promptToLocalize ){
        return localizationTable.getProperty( promptToLocalize, "<<error - not localized>>");
    }
    
}
