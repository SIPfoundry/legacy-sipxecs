/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */

package org.sipfoundry.translator;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.util.Properties;

import com.google.api.translate.Language;
import com.google.api.translate.Translate;

public class Translator {

    
    public static void main(String[] args) throws Exception {
         Translate.setHttpReferrer("http://www.sipfoundry.org");
         
         String propertiesFile = System.getProperty("source.file");
         
         String targetLanguageTxt = System.getProperty("target.language");
         
         String sourceLanguageTxt = System.getProperty("source.language");
          
         String translatedFile = System.getProperty("target.file");
         
         Language sourceLanguage = Language.valueOf(sourceLanguageTxt);
         if ( sourceLanguage == null ) {
             throw new IllegalArgumentException(sourceLanguageTxt);
         }
         
         Language targetLanguage = Language.valueOf(targetLanguageTxt);
         
         if ( targetLanguage == null ) {
             throw new IllegalArgumentException(targetLanguageTxt);
         }
         
         Properties props = new Properties();
         
         props.load(new FileInputStream( new File(propertiesFile)));
         
         PrintWriter printWriter = translatedFile != null ?
                  new PrintWriter( new File(translatedFile)): new PrintWriter(System.out);
                  
        
         for ( Object name  : props.keySet() ) {
             String sourceLanguageString = props.getProperty((String) name);
             String translatedText = Translate.execute(sourceLanguageString, sourceLanguage, targetLanguage);
             String newPropLine = name + "=" + translatedText;
             printWriter.println(newPropLine);

         }
         printWriter.flush();
         printWriter.close();
         
    }

}
