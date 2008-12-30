package com.pingtel.sipviewer;

import java.applet.Applet;
import java.net.URL;

import javax.swing.JOptionPane;

public class SIPViewerApplet extends Applet {
    SIPViewerFrame frame ;
    
    
    @Override
    public void init() {
      try {
        String  fileName = getParameter("TRACE-FILE");
        frame = new SIPViewerFrame(false) ;
        URL documentBase = this.getDocumentBase();
        String docBaseStr = documentBase.toExternalForm();
        String traceFileStr = docBaseStr + "/" + fileName ;
        frame.applySourceFile(new URL( traceFileStr) );
      } catch (Exception ex) {
          JOptionPane.showConfirmDialog(null, "Could not open trace file ",  "Error", 
                  JOptionPane.DEFAULT_OPTION, JOptionPane.ERROR_MESSAGE, null);
      }
    }
    
    @Override
    public void show() {
        frame.setVisible(true);
    }
    
    @Override
    public void hide() {
        frame.setVisible(false);
    }
    

}
