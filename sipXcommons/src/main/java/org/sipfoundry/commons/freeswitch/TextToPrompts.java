/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.freeswitch;

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/**
 * Convert a variable into a list of prompt files that expresses that value as audio. A value is
 * text that can be represented as various numerical types. Each type can have additional
 * formatting applied (such as the ability to render times in 24 hour "military" time, or with
 * AM/PM). <br>
 * <br>
 * Extend this abstract class and implement the various type conversions for each locale. The
 * class must be named sipxivr.TextToPrompts_{locale}.class. <br>
 * <br>
 * The getTextToPrompt() method will search for and instantiate the most specific class based on
 * the local using classForName() at run time. If no class is found, it will default to
 * sipxivr.TextToPrompts_en.class
 *
 */
public abstract class TextToPrompts {

    public enum Types {
        cardinal, ordinal, digits, letters, prompts, date, pause
    }

    private String m_prefix; // The prefix for the audio files used on this locale
    private Types m_type; // The type of value to render
    private String m_format; // Type specific formatting string
    private String m_value; // The value

    /**
     * Instantiate the appropriate TextToPrompts object for the given locale.
     *
     * @param l
     * @return
     */
    public static TextToPrompts getTextToPrompt(Locale l, String className) {
        TextToPrompts foundClass = null;

        String lang = l.getLanguage();
        String country = l.getCountry();
        String variant = l.getVariant();

        String thisClassName = TextToPrompts.class.getCanonicalName();

        if(className != null) {
            Class< ? > c;
            try {
                String fullClassName = thisClassName;
                int index = fullClassName.indexOf("TextToPrompts");
                fullClassName = fullClassName.substring(0, index) + className;              
                
                c = Class.forName(fullClassName);
                foundClass = (TextToPrompts) c.newInstance();
                return foundClass;
            } catch (Throwable t) {
                // keep going
            }
        }
        
        // Search for the appropriate class, most generic first.
        // Keep getting more specific until an error is thrown.
        try {           
            Class< ? > c = Class.forName(thisClassName + "_" + lang);
            foundClass = (TextToPrompts) c.newInstance();
            c = Class.forName(thisClassName + "_" + lang + "_" + country);
            foundClass = (TextToPrompts) c.newInstance();
            c = Class.forName(thisClassName + "_" + lang + "_" + country + " " + variant);
            foundClass = (TextToPrompts) c.newInstance();
        } catch (Throwable t) {
            if (foundClass == null) {
            	// TODO: Add logging support.
                // LOG.error("Cannot find TextToPrompt subclass for locale " + l.toString() + ".  Using locale en as backup.");
                foundClass = new TextToPrompts_en();
            }
        }

        return foundClass;
    }

    /**
     * Set the prompt prefix
     *
     * @param prefix
     */
    public void setPrefix(String prefix) {
        if (!prefix.endsWith("/")) {
            prefix += "/";
        }
        this.m_prefix = prefix;
    }

    public void setType(String type) {
        this.m_type = Types.valueOf(type);
    }

    public void setType(Types type) {
        this.m_type = type;
    }

    public void setFormat(String format) {
        this.m_format = format;
    }

    public String getFormat() {
        return m_format;
    }

    /**
     * Render the value as a colon separated list of audio files to play that "speaks" the value
     * as if were type, in a format. <br>
     * <br>
     * Value to speak is the member variable "value".
     *
     * @return
     */
    public String render() {
        switch (m_type) {
        case cardinal:
            return cardinal();
        case ordinal:
            return ordinal();
        case digits:
            return digits();
        case letters:
            return letters();
        case prompts:
            return prompts();
        case date:
            return date();
        case pause:
        	return pause();
        default:
            break;
        }

        return null;
    }

    /**
     * Render the value as a colon separated list of audio files to play that "speaks" the value
     * as if were type, in a format. <br>
     * <br>
     * Value to speak is passed in.
     *
     * @param value
     * @return
     */
    public String render(String value) {
        this.m_value = value;
        return render();
    }

    /**
     * Render the value as a colon separated list of audio files to play that "speaks" the value
     * as if were type. <br>
     * <br>
     * Value to speak, type, and format are passed in.
     *
     * @param value
     * @return
     */
    public String render(String value, Types type, String format) {
        this.m_value = value;
        this.m_type = type;
        this.m_format = format;
        return render();
    }

    /**
     * Append the path prefix to each un-rooted file in a colon separated list of files.
     *
     * @param prompts A colon separated list of files.
     * @return A colon separated list of files
     */
    public String appendPrefix(String prompts) {
        if (m_prefix == null || prompts == null || prompts.contentEquals("")) {
            return prompts;
        }

        StringBuilder results = new StringBuilder("");
        String[] promptArray = prompts.split(":");
        for (String prompt : promptArray) {
            // If rooted, don't prepend prefix
            if (!prompt.startsWith("/") && !prompt.startsWith(".")) {
                prompt = m_prefix + prompt;
            }
            results.append(prompt).append(":");
        }
        // remove trailing ":"
        return results.substring(0, results.length() - 1);
    }

    /**
     * Render the value as a cardinal. (A cardinal number is a counting number, like "one hundred
     * and two")
     *
     * @return
     */
    public abstract String cardinal();

    /**
     * Render the value as an ordinal. (An ordinal number is positional, like "first, second,
     * third".)
     *
     * @return
     */
    public abstract String ordinal();

    /**
     * Render the value as individual digits.
     * (Non digits in value are silently ignored)
     *
     * @return
     */
    public abstract String digits();

    /**
     * Render the value as individual letters.
     * (characters for which there are no prompts are silently ignored)
     *
     * @return
     */
    public abstract String letters();

    /**
     * Render the value as a colon separated list of prompts.  Just returns value unchanged.
     * (Allows for passing pre-rendered or pre-recorded files in place of a value)
     *
     * @return
     */
    public abstract String prompts();

    public String pause() {
    	int ms;
    	
    	if (getValue().equals("-")) {
    		ms = Integer.parseInt(getFormat());
    	} else {
           ms = Integer.parseInt(getValue());
    	}
    
        String result = String.format("silence_stream://%d", ms);
        return result;
    }

    public String getValue() {
        return m_value;
    }

    protected abstract String time12(Date d);

    protected abstract String time24(Date d);

    protected abstract String dayOfWeek(Date d);

    protected abstract String dayOfMonth(Date d);

    protected abstract String month(Date d);

    protected abstract String year(Date d);

    protected abstract String ohJoy(int num);

    /**
     * The Date format needed for feeding date
     * @return
     */
    public static DateFormat ttsDateFormat() {
        return new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
    }

    /**
     * Render the value as a Date and/or Time.
     * <br><br>
     * The input format is fixed: yyyy-MM-dd HH:mm:ss
     * <br><br>
     * The output format varies based on m_format, which is a list of single characters:
     * <br>
     * <br>Y = year
     * <br>M = month
     * <br>D = Day of month (an ordinal 1st, 2nd, 3rd...)
     * <br>W = Day of week (Monday, Tuesday...)
     * <br>m = Time (hours and minutes) in 24 hour "military" style
     * <br>T = Time (hours and minutes) in AM/PM style
     * <br>
     * <br>For example, if m_value = "1963-01-26 13:42:11", and m_format = "WMDYT", and the locale is en_US
     * <br>The output would be:
     * <br>saturday.wav:january.wav:26th.wav:19.wav:63.wav:01.wav:42.wav:pm.wav
     * @return
     */
    public String date() {
        String result = "";

        try {
            // Parse value as YYYY-MM-DD HH:MM:SS
            DateFormat timeFormat = ttsDateFormat();
            Date d = timeFormat.parse(getValue());

            // For each letter of format, get the corresponding part of the value
            for (int i=0; i< getFormat().length(); i++) {
                if (result.length() > 0) {
                    result += ":";
                }

                switch (getFormat().charAt(i)) {
                case 'Y': // Year
                    result += year(d);
                    break;
                case 'M': // Month
                    result += month(d);
                    break;
                case 'D': // Day of month (an ordinal, 1st, 2nd, 3rd, etc.)
                    result += dayOfMonth(d);
                    break;
                case 'W': // Day of week (Monday, Tuesday, etc.)
                    result += dayOfWeek(d);
                    break;
                case 'm': // Time (hours & minutes) in 24 hour "military" style "Oh one hundred hours"
                    result += time24(d);
                    break;
                case 'T': // Time (hours & minutes) in AM/PM style "twelve oh five, PM"
                    result += time12(d);
                    break;
                default:
                    break;
                }
            }
        } catch (ParseException e) {
        	// TODO: Add logging support
            // LOG.error("Value of for date() is not in yyyy-MM-dd HH:mm:ss format!.  Value="+getValue());
        }
        return appendPrefix(result);
    }

}
