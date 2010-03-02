package com.pingtel.sipviewer;

import java.awt.AWTException;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

import javax.imageio.ImageIO;
import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.border.Border;
import javax.swing.filechooser.FileNameExtensionFilter;

public class PopUpUtils
{

    // pop-up menu titles for the main display screen
    static String Item1 = "Capture Screen (2 sec delay)";
    static String Item2 = "Set Dialog Background";
    static String Item3 = "Keep Only This Dialog";
    static String Item4 = "Remove Dialog";

    // pop-up menu titles for the time index screens
    static String Item11 = "Date and Time";
    static String Item22 = "Time of Day (Default)";
    static String Item33 = "Since Previous Captured Message";
    static String Item44 = "Since Beginning of Capture";
    static String Item55 = "Since Key Index";
    static String Item66 = "Set Key Index";

    // stores the time key index for the "Since Key Index
    // (Set Key Index)" Time Display Format
    static int keyIndex = 0;

    // stores the x location for drawing the time index
    // values in the time index column, in order for the
    // numbers to be vertically aligned with the decimal
    // being the guide this value is used and it varies
    // depending on the time display format selected
    static int xTimeIndexOffset = 0;

    // these are the different time display format modes
    static enum TimeDisplayMode {
        DATE_AND_TIME, TIME_OF_DAY_DEFAULT, SINCE_PREVIOUS, SINCE_BEGINNING, SINCE_KEY_INDEX
    };

    // stores the currently selected time display format
    static TimeDisplayMode currentTimeDisplaySelection = TimeDisplayMode.TIME_OF_DAY_DEFAULT;

    public static void captureScreen(SIPViewerFrame frameRef)
    {
        // Create a file chooser
        JFileChooser fc = new JFileChooser();
        FileNameExtensionFilter filter = new FileNameExtensionFilter("PNG Images", "png");
        fc.setFileFilter(filter);

        // Creating a buffer for our image
        BufferedImage image = null;

        try
        {
            // Capture the screen, make sure to capture before we display the
            // "save" file dialog so that "save" file dialog doesn't eclipse
            // sipviewer window
            image = new Robot().createScreenCapture(new Rectangle(frameRef.getX(), frameRef.getY(),
                    frameRef.getWidth(), frameRef.getHeight()));
        } catch (AWTException e)
        {
            // e.printStackTrace();
        }

        // Show the "save" file dialog
        int returnVal = fc.showSaveDialog(frameRef);

        // if user pressed save lets do the input checking
        if (returnVal == JFileChooser.APPROVE_OPTION)
        {
            File fileName = fc.getSelectedFile();

            // if user actually entered something process the input
            if ((fileName.getName() != null) && (fileName.getName().length() > 0))
            {
                // file must end with a png extension, make user type it in
                if (!fileName.getName().toLowerCase().endsWith(".png"))
                {
                    JOptionPane.showMessageDialog(null, "Error: file name must end with \".png\".",
                            "Wrong file name", 1);
                }
                else
                {
                    // all looks good lets save the file
                    try
                    {
                        ImageIO.write(image, "png", new File(fileName.getAbsolutePath()));
                        JOptionPane.showMessageDialog(null, "Screen captured successfully.",
                                "SipViewer Screen Capture", 1);
                    } catch (IOException e)
                    {
                        // e.printStackTrace();
                    }
                }
            }
        }
        else
        {
            // user canceled the request do nothing
        }
    }

    // this dialog shows the colors available for setting the sip dialog
    // backgrounds
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
        Color color1 = new Color(64, 46, 46);
        Color color2 = new Color(129, 2, 2);
        Color color3 = new Color(1, 84, 3);
        Color color4 = new Color(66, 68, 66);
        Color color5 = new Color(22, 51, 60);
        Color color6 = new Color(23, 22, 60);
        Color color7 = new Color(47, 22, 60);

        // current selected color, this is reset before the dialog is set to
        // visible in the ChartBody, if user exits without selecting a color
        // it will remain null
        Color selectedColor;

        // class constructor
        public ColorChooserDialog(JFrame jf) {
            // assigning title and setting modal to true, this way user has to
            // make a selection or dismiss the dialog to continue using
            // sipviewer
            super(jf, "Color Chooser", true);

            // setting layout and constructing all the buttons
            setLayout(new GridLayout(0, 1));

            ButtonGroup boxOfColors = new ButtonGroup();
            Border border = BorderFactory.createEmptyBorder(4, 4, 4, 4);

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

            setSize(200, 300);
        }

        // creates the button object sets all the attibutes
        protected JButton createColor(String name, Border normalBorder, Color buttonColor)
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
        public void actionPerformed(ActionEvent e)
        {

            String command = ((JButton) e.getSource()).getActionCommand();
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

    // this method recalculates the time index values based
    // on the current time display format selection, it stores
    // these values with each entry so they can be easy to
    // repaint and don't have to be recalculated for each
    // repaint operation
    public static void setTimeIndex(SIPViewerFrame m_frame)
    {
        int iEntries = m_frame.m_model.getSize();
        int iNumKeys = m_frame.m_model.getNumKeys();

        // stores the previous time index value so that
        // deltas can be calculated
        long timeMicrosecondsPrevious = 0;
        int columnWidth = 0;

        // setting up the formatter for formating the time
        // index values
        NumberFormat formatter = new DecimalFormat("#0.000000");
        ChartDescriptor keyEntry = null;

        // set the column width of the time index column based on the
        // time display format selected, we also set the radio button
        // in the SIPViewerFrame Time Display Format menu and lastly
        // the x time index offset for displaying the time index values
        // (x time index is used to align the time values vertically)
        // Note: the numbers here were arrived through experimentation
        // to get the best look, feel and alignment
        if (currentTimeDisplaySelection == TimeDisplayMode.DATE_AND_TIME)
        {
            columnWidth = 227;
            m_frame.m_dateAndTimeFormat.setSelected(true);
            xTimeIndexOffset = 165;
        }
        else if (currentTimeDisplaySelection == TimeDisplayMode.TIME_OF_DAY_DEFAULT)
        {
            columnWidth = 152;
            m_frame.m_defaultTimeFormat.setSelected(true);
            xTimeIndexOffset = 90;
        }
        else if (currentTimeDisplaySelection == TimeDisplayMode.SINCE_PREVIOUS)
        {
            columnWidth = 123;
            m_frame.m_sincePreviousFormat.setSelected(true);
            xTimeIndexOffset = 62;
        }
        else if (currentTimeDisplaySelection == TimeDisplayMode.SINCE_BEGINNING)
        {
            columnWidth = 123;
            m_frame.m_sinceBeginningFormat.setSelected(true);
            xTimeIndexOffset = 62;
        }
        else if (currentTimeDisplaySelection == TimeDisplayMode.SINCE_KEY_INDEX)
        {
            columnWidth = 123;
            m_frame.m_sinceKeyIndexFormat.setSelected(true);

            // this mode also requires to set the keyEntry that corresponds
            // to the keyIndex that user set (default is location 0), so that
            // we can use the entry to calculate the negative previous
            // cumulative deltas as well as forward positive cumulative deltas
            keyEntry = (ChartDescriptor) m_frame.m_model.getEntryAt(keyIndex);
            xTimeIndexOffset = 62;
        }

        // lets set the column width for both time index columns even if the
        // second time index column is not visible (even if user is working
        // in a single screen mode)
        m_frame.m_scrollPaneTimeIndex.setMinimumSize(new Dimension(columnWidth, 50));
        m_frame.m_scrollPaneTimeIndexSecond.setMinimumSize(new Dimension(columnWidth, 50));

        // loop over all the elements and recalculate the time index values
        if ((iEntries > 0) && (iNumKeys > 0))
        {
            for (int i = 0; i < iEntries; i++)
            {
                if ((i >= 0) && (i < m_frame.m_model.getSize()))
                {
                    // getting current entry
                    ChartDescriptor entry = (ChartDescriptor) m_frame.m_model.getEntryAt(i);

                    // we only deal with entries that are visible to the user
                    if (entry.displayIndex >= 0)
                    {
                        // current time display format is set to date and time
                        if (currentTimeDisplaySelection == TimeDisplayMode.DATE_AND_TIME)
                        {
                            // if we are using local time zone then we need to
                            // convert the time to local time zone values
                            if (m_frame.m_localTimeZone.isSelected())
                            {
                                // getting instance of our local calendar
                                GregorianCalendar localCalendar = new GregorianCalendar();

                                // setting the calendar time to UTC time, in
                                // milliseconds
                                localCalendar
                                        .setTimeInMillis(entry.dataSource.timeStampInMicroseconds / 1000);

                                // getting an instance of a date set with UTC
                                // time and accounting for the time zone and
                                // day light savings offsets
                                Date localTime = new Date(
                                        (entry.dataSource.timeStampInMicroseconds / 1000)
                                                + localCalendar.get(Calendar.ZONE_OFFSET)
                                                + localCalendar.get(Calendar.DST_OFFSET));

                                // setting date and time formatting to be
                                // consistent with default UTC display
                                SimpleDateFormat sdf = new SimpleDateFormat(
                                        "yyyy-MM-dd HH:mm:ss.SSS");

                                // decimal format used to append the
                                // microseconds time to the string created
                                // with the sdf above
                                DecimalFormat df = new DecimalFormat("000");

                                // setting the value formated with appended
                                // microseconds
                                entry.dataSource.timeIndexDisplay = sdf.format(localTime)
                                        + df.format(entry.dataSource.timeStampThreeDigitAccuracy);
                            }
                            else
                            {
                                // UTC we can just grab from the raw format that
                                // we got from the log file with getting rid
                                // of the letter 'T' that is in the middle
                                entry.dataSource.timeIndexDisplay = entry.dataSource.timeStamp
                                        .substring(0, 26).replace('T', ' ');
                            }
                        }
                        else if (currentTimeDisplaySelection == TimeDisplayMode.TIME_OF_DAY_DEFAULT)
                        {
                            // if we are using local time zone then we need to
                            // convert the time to local time zone values
                            if (m_frame.m_localTimeZone.isSelected())
                            {
                                // getting instance of our local calendar
                                GregorianCalendar localCalendar = new GregorianCalendar();

                                // setting the calendar time to UTC time, in
                                // milliseconds
                                localCalendar
                                        .setTimeInMillis(entry.dataSource.timeStampInMicroseconds / 1000);

                                // getting an instance of a date set with UTC
                                // time and accounting for the time zone and day
                                // light savings offsets
                                Date localTime = new Date(
                                        (entry.dataSource.timeStampInMicroseconds / 1000)
                                                + localCalendar.get(Calendar.ZONE_OFFSET)
                                                + localCalendar.get(Calendar.DST_OFFSET));

                                // setting time of day formatting to be
                                // consistent with UTC display
                                SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss.SSS");

                                // decimal format used to append the
                                // microseconds time
                                DecimalFormat df = new DecimalFormat("000");

                                // setting the value formated with appended
                                // microseconds
                                entry.dataSource.timeIndexDisplay = sdf.format(localTime)
                                        + df.format(entry.dataSource.timeStampThreeDigitAccuracy);
                            }
                            else
                            {
                                // UTC we can just grab from the raw format that
                                // we got from the log file
                                entry.dataSource.timeIndexDisplay = entry.dataSource.timeStamp
                                        .substring(entry.dataSource.timeStamp.indexOf('T') + 1,
                                                entry.dataSource.timeStamp.length() - 1);
                            }
                        }
                        else if (currentTimeDisplaySelection == TimeDisplayMode.SINCE_PREVIOUS)
                        {

                            // for the first entry we set the previous
                            // microseconds value to that of the actual first
                            // message so we'll start with 0.000000 value after
                            // calculations are done
                            if (entry.displayIndex == 0)
                                timeMicrosecondsPrevious = entry.dataSource.timeStampInMicroseconds;

                            // lets subtract the previous time value from the
                            // current one to get the delta
                            Long microsecondSubtraction = entry.dataSource.timeStampInMicroseconds
                                    - timeMicrosecondsPrevious;

                            // now we convert the microsecond value to a correct
                            // decimal point representation
                            Double conversion = Double.valueOf(microsecondSubtraction) / 1000000;

                            // here we store the current time index value to be
                            // used in calculation in the next iteration
                            timeMicrosecondsPrevious = entry.dataSource.timeStampInMicroseconds;

                            // lets store the final formatted value
                            entry.dataSource.timeIndexDisplay = formatter.format(conversion);
                        }
                        else if (currentTimeDisplaySelection == TimeDisplayMode.SINCE_BEGINNING)
                        {
                            // we are doing a cumulative time progression so we
                            // set the key entry (from which to start
                            // accumulating time) to the very top first entry
                            if (entry.displayIndex == 0)
                                keyEntry = entry;

                            // getting a delta between the key entry and the
                            // current entry
                            Long microsecondSubtraction = entry.dataSource.timeStampInMicroseconds
                                    - keyEntry.dataSource.timeStampInMicroseconds;

                            // converting to decimal point representation
                            Double conversion = Double.valueOf(microsecondSubtraction) / 1000000;

                            // storing the final version of the string
                            entry.dataSource.timeIndexDisplay = formatter.format(conversion);
                        }
                        else if (currentTimeDisplaySelection == TimeDisplayMode.SINCE_KEY_INDEX)
                        {
                            // the key index value has been already set by the
                            // user so we just use it to perform the
                            // calculation, format and set the time index string
                            Long microsecondSubtraction = entry.dataSource.timeStampInMicroseconds
                                    - keyEntry.dataSource.timeStampInMicroseconds;

                            Double conversion = Double.valueOf(microsecondSubtraction) / 1000000;

                            entry.dataSource.timeIndexDisplay = formatter.format(conversion);

                        }
                    }
                }
            }
        }
    }
}
