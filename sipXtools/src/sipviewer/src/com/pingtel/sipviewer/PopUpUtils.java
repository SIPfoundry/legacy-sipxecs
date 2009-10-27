package com.pingtel.sipviewer;

import java.awt.AWTException;
import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.filechooser.FileNameExtensionFilter;

public class PopUpUtils {

    public static void captureScreen(SIPViewerFrame frameRef) {
        // Create a file chooser
        JFileChooser fc = new JFileChooser();
        FileNameExtensionFilter filter = new FileNameExtensionFilter("PNG Images", "png");
        fc.setFileFilter(filter);

        // Creating a buffer for our image
        BufferedImage image = null;

        try {
            // Capture the screen, make sure to capture before we display the
            // "save" file
            // dialog so that "save" file dialog doesn't eclipse sipviwer window
            image = new Robot().createScreenCapture(new Rectangle(frameRef.getX(), frameRef.getY(), frameRef
                    .getWidth(), frameRef.getHeight()));
        } catch (AWTException e) {
            // e.printStackTrace();
        }

        // Show the "save" file dialog
        int returnVal = fc.showSaveDialog(frameRef);

        // if user pressed save lets do the input checking
        if (returnVal == JFileChooser.APPROVE_OPTION) {
            File fileName = fc.getSelectedFile();

            // if user actually entered something process the input
            if ((fileName.getName() != null) && (fileName.getName().length() > 0)) {
                // file must end with a png extension, make user type it in
                if (!fileName.getName().toLowerCase().endsWith(".png")) {
                    JOptionPane.showMessageDialog(null, "Error: file name must end with \".png\".",
                            "Wrong file name", 1);
                } else {
                    // all looks good lets save the file
                    try {
                        ImageIO.write(image, "png", new File(fileName.getAbsolutePath()));
                        JOptionPane.showMessageDialog(null, "Screen captured successfully.",
                                "SipViewer Screen Capture", 1);
                    } catch (IOException e) {
                        // e.printStackTrace();
                    }
                }
            }
        } else {
            // user canceled the request do nothing
        }
    }

}
