package com.pingtel.sipviewer;

import java.awt.AWTException;
import java.awt.Color;
import java.awt.Font;
import java.awt.GridLayout;
import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;
import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JToggleButton;
import javax.swing.border.Border;
import javax.swing.filechooser.FileNameExtensionFilter;

public class PopUpUtils {
		
	static String Item1   = "Capture Screen (2 sec delay)";
	static String Item2   = "Set Dialog Background";
	static String Item3   = "Keep Only This Dialog";
	static String Item4   = "Remove Dialog";
	
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
    
    // this dialog shows the colors available for setting the sip dialog backgrounds 
    static public class ColorChooserDialog extends JDialog implements ActionListener 
    {
    	// color buttons
    	JButton buttonBlackColor;
        JButton buttonColor1;
        JButton buttonColor2;
        JButton buttonColor3;
        JButton buttonColor4;
        JButton buttonColor5;
        JButton buttonColor6;
        JButton buttonColor7;
        
        // actuall color values
        Color color1 = new Color(64,46,46);
        Color color2 = new Color(129,2,2);
        Color color3 = new Color(1,84,3);
        Color color4 = new Color(66,68,66);
        Color color5 = new Color(22, 51, 60);
        Color color6 = new Color(23, 22, 60);
        Color color7 = new Color(47, 22, 60);

        // current selected color, this is reset before the dialog is set to
        // visible in the ChartBody, if user exits without selecting a color
        // it will remain null
        Color selectedColor;
    	
        // class constructor
    	public ColorChooserDialog (JFrame jf)
    	{
    		// assigning title and setting modal to true, this way user has to
    		// make a selection or dismiss the dialog to continue using sipviewer 
    		super(jf, "Color Chooser", true);
    		
    		// setting layout and constructing all the buttons
    		setLayout(new GridLayout(0, 1));

    	    ButtonGroup boxOfColors = new ButtonGroup();
    	    Border border = BorderFactory.createEmptyBorder(4,4,4,4);

    	    buttonBlackColor = createColor("black", border, Color.BLACK);
    	    boxOfColors.add(buttonBlackColor);
    	    add(buttonBlackColor);

    	    buttonColor1 = createColor("color1", border, color1);
   	     	boxOfColors.add(buttonColor1);
   	     	add(buttonColor1);
    	     
    	    buttonColor2 = createColor("color2", border, color2);
    	    boxOfColors.add(buttonColor2);
    	    add(buttonColor2);
    	     
    	    buttonColor3 = createColor("color3", border, color3);
    	    boxOfColors.add(buttonColor3);
    	    add(buttonColor3);
    	     
    	    buttonColor4 = createColor("color4", border, color4);
    	    boxOfColors.add(buttonColor4);
    	    add(buttonColor4);
    	     
    	    buttonColor5 = createColor("color5", border, color5);
    	    boxOfColors.add(buttonColor5);
    	    add(buttonColor5);
    	     
    	    buttonColor6 = createColor("color6", border, color6);
    	    boxOfColors.add(buttonColor6);
    	    add(buttonColor6);
    	     
    	    buttonColor7 = createColor("color7", border, color7);
    	    boxOfColors.add(buttonColor7);
    	    add(buttonColor7);
    	     
    		setSize(200,300);
    	}
    	
    	// creates the button object sets all the attibutes
        protected JButton createColor(String name,
                Border normalBorder, Color buttonColor) 
        {
        	JButton colorButton = new JButton();
        	colorButton.setActionCommand(name);
        	colorButton.setBackground(buttonColor);
        	colorButton.setHorizontalAlignment(JButton.HORIZONTAL);
        	colorButton.addActionListener(this);

        	return colorButton;
        }
        
        // handles the events, if user clicks on one of the buttons
        // the selectedColor is assigned its value and the color chooser
        // dialog disappears
        public void actionPerformed(ActionEvent e) {

            String command = ((JButton)e.getSource()).getActionCommand();
            if ("black".equals(command))
            	selectedColor = Color.black;
            else if ("color1".equals(command))
            	selectedColor = color1;
            else if ("color2".equals(command))
            	selectedColor = color2;
            else if ("color3".equals(command))
            	selectedColor = color3;
            else if ("color4".equals(command))
            	selectedColor = color4;
            else if ("color5".equals(command))
            	selectedColor = color5;
            else if ("color6".equals(command))
            	selectedColor = color6;
            else if ("color7".equals(command))
            	selectedColor = color7;
            
            this.setVisible(false);
        }

        // method used by ChartBody to reset the current selected color
        // variable, by calling this before showing the dialog we will
        // know when dialogue is dismissed wheater a color was selected
        // or not
        public void resetSelectedColor()
        {
        	selectedColor = null;
        }
        
        // used to get the value of the selected color.
        public Color getSelectedColor()
        {
        	return selectedColor;
        }
    }
}
