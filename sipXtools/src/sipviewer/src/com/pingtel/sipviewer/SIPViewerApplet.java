/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package com.pingtel.sipviewer;

import java.applet.Applet;
import java.awt.Graphics;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.net.URL;

import javax.swing.JApplet;
import javax.swing.JButton;
import javax.swing.JOptionPane;

public class SIPViewerApplet extends JApplet {
    SIPViewerFrame frame ;
    
    
    @Override
    public void init() {
      try {
        String  fileName = getParameter("MERGED-XML");
       
        frame = new SIPViewerFrame(false) ;
        URL codeBase = this.getCodeBase();
        String baseStr = codeBase.toExternalForm();
        String traceFileStr = baseStr + "/" + fileName ;
        frame.applySourceFile(new URL( traceFileStr) );
        JButton b = new JButton("Click to see call flow " );
        this.add(b);
        
        b.addActionListener( new ActionListener() {
            public void actionPerformed(ActionEvent arg0) {
                if ( frame.isVisible() ) {
                    frame.setVisible(false);
                } else {
                    frame.setVisible(true);
                }
                
            } });
      } catch (Exception ex) {
          ex.printStackTrace();
          JOptionPane.showConfirmDialog(null, "Could not open trace file ",  "Error", 
                  JOptionPane.DEFAULT_OPTION, JOptionPane.ERROR_MESSAGE, null);
      }
    }
    
    @Override
    public void paint(Graphics g) {
        super.paint(g);
    }
    
    
    
    @Override
    public void hide() {
        if ( frame != null ) {
            frame.setVisible(false);
        }
    }
    

}
