package org.sipfoundry.sipximbot;


import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Properties;

import org.apache.log4j.Logger;

public class Localizer
{
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipximbot");
    
    private Properties localizationTable;

    Localizer() {
        
        String localizationFile = System.getProperty("localization.file",  System.getProperty("conf.dir", "/etc/sipxpbx") + "/imbot/sipximbot-prompts.properties");
        Properties props = new Properties();
        try {
            props.load(new FileInputStream(localizationFile));
        } catch (FileNotFoundException e){
            LOG.warn("Localizer cannot find localization file '" + localizationFile + "'");
        } catch (IOException e) {
            LOG.error("Localizer caught " + e );
        }
        
        localizationTable = props;
    }

    String localize(String promptToLocalize ){
        return localizationTable.getProperty( promptToLocalize + ".prompt", "<<error - not localized>>");
    }

}

