/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package com.pingtel.sipviewer ;

import javax.swing.* ;
import java.io.* ;

/**
 * SIPViewer application entry point
 */
public class SIPViewer
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final String CMDLINE_ALIASES = "-A" ;
    public static final String CMDLINE_HELP = "-" ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    public static String s_strAliasesFile ;
    public static String s_strCmdLineFile ;


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public static void main(String args[]) throws Exception  {
        if (processCmdlineArgs(args))
        {        	        	
        	// check that file exists
        	if (s_strCmdLineFile != null)
        	{
        		File file = new File(s_strCmdLineFile);
        	
        		// if file does not exist then let the user know and exit
        		if (!file.exists())
        		{        			        	
        			System.out.println();
        			System.out.print("File not found: " + s_strCmdLineFile);
        			System.out.println();
        			usage();
        			return;
        		}
        	}
        	        	
            SIPViewerFrame frame = new SIPViewerFrame(true) ;
            if (s_strAliasesFile != null)
                frame.applyAliasesFile(s_strAliasesFile) ;
            if (s_strCmdLineFile != null) { 
                frame.applySourceFile(s_strCmdLineFile) ;                
            }
            
            frame.setVisible(true);
            
            // command line invocation causes frame characteristics to
            // be skewed until the frame is actually shown on the screen so
            // we don't actually process any information untill the window is
            // shown then call the reload
            frame.m_Reload.execute();
        }
        else
        {
            usage() ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    protected static boolean processCmdlineArgs(String args[])
    {
        boolean bError = false ;
        for (int i=0; i<args.length; i++)
        {
            String f ;

            if (args[i].toUpperCase().startsWith(CMDLINE_ALIASES))
            {
                if (s_strAliasesFile == null)
                    s_strAliasesFile = args[i].substring(CMDLINE_ALIASES.length()) ;
                else
                    bError = true ;

            }
            else if (args[i].startsWith(CMDLINE_HELP))
            {
                bError = true ;
            }
            else
            {
                if (s_strCmdLineFile == null)
                    s_strCmdLineFile = args[i] ;
                else
                    bError = true ;
            }
        }

        return !bError ;
    }

    protected static void usage()
    {
        System.out.println() ;
        System.out.println("SIPViewer" +
                " ["+CMDLINE_ALIASES+"<aliases file>]" +
                " [<sip trace file>]") ;
        System.out.println() ;
    }
}
