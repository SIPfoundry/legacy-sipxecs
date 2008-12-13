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
    public static void main(String args[])
    {
        if (processCmdlineArgs(args))
        {
            SIPViewerFrame frame = new SIPViewerFrame() ;
            if (s_strAliasesFile != null)
                frame.applyAliasesFile(s_strAliasesFile) ;
            if (s_strCmdLineFile != null)
                frame.applySourceFile(s_strCmdLineFile) ;
            frame.show() ;
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
