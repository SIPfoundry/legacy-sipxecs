/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.freeswitch;

import java.util.Calendar;
import java.util.Date;

/**
 * Convert a variable into a list of prompt files that expresses that value as audio. A value is
 * text that can be represented as various numerical types. Each type can have additional
 * formatting applied (such as the ability to render times in 24 hour "military" time, or with
 * AM/PM). <br>
 * <br>
 * This is the English version. It uses a prompt set from the sipXecs standard prompts library.
 * The prompt names are hard-coded to match those prompts.
 *
 *
 */
public class TextToPromptsCP extends TextToPrompts {

    /**
     * Convert cardinal numbers 1000 or less
     *
     * @param card
     * @return
     */
    private String smallCardinal(int card) {

        if(card == 0) {
            return "0877.wav";
        }
        if (card > 99) {
            // "more than ninety-nine"
            return "0802.wav";
        }
       
        return String.format("%04d.wav", card);
    }

    /**
     * Convert positive cardinal numbers up to 999,999
     *
     * @param card
     * @return
     */
    private String largeCardinal(int card) {


        if (card > 99) {
            return digits();
        }

        // Values from 0-99 are easy!
        return smallCardinal(card);
    }

    /**
     * Convert a small Ordinal (small is 31 or less!)
     *
     * @param ord
     * @return
     */
    private String smallOrdinal(int ord) {

        // Easy to calculate
        if (ord <= 31) {
            return String.format("%04d.wav", ord + 159);
        }

        return null;
    }

    /**
     * Convert signed cardinal numbers to prompt files
     */
    @Override
    public String cardinal() {
        String result = "";

        int origValue = 0;

        try {
            origValue = Integer.parseInt(getValue());
        } catch (Throwable t) {
            return result;
        }

        int v = origValue;

        if (v < 0) {
            v = -v; // Convert back to positive
        }
        return appendPrefix(result + largeCardinal(v));
    }

    @Override
    public String ordinal() {
        int origValue = 0;

        try {
            origValue = Integer.parseInt(getValue());
        } catch (Throwable t) {
            return "";
        }

        // Cannot handle ordinals greater than 31
        if (origValue <= 0 || origValue > 31) {
            return "";
        }

        // Handle the easy cases
        String result = smallOrdinal(origValue);

        if (result == null) {
            // Frankenstein 'em
            int tenths = origValue / 10;
            result = smallCardinal(tenths * 10) + ":" + smallOrdinal(origValue % 10);
        }

        return appendPrefix(result);
    }

    /**
     * Speak string of digits (ignore non-digits)
     *
     */
    @Override
    public String digits() {
        String result = "";

        for (int i = 0; i < getValue().length(); i++) {
            String l = getValue().substring(i, i + 1);
            if ("0123456789".contains(l)) {
                result += smallCardinal(Integer.parseInt(l)) + ":";
            }
        }

        if (result.contentEquals("")) {
            return "";
        }

        // remove trailing ":"
        result = result.substring(0, result.length() - 1);

        return appendPrefix(result);
    }

    /**
     * Speak string of letters and symbols
     *
     */
    @Override
    public String letters() {
        String result = "";

        String v = getValue().toLowerCase();
        int len = v.length();
        
        for (int i = 0; i < len; i++) {
            String l = v.substring(i, i + 1);
            if ("0123456789".contains(l)) {
                // if the last digit then play with a descending intonation
                if(i == len-1) {
                    result += String.format("%04d.wav", Integer.parseInt(l) + 295) + ":";
                } else {
                    result += String.format("%04d.wav", Integer.parseInt(l) + 278) + ":";
                }
            }
            
            if(l.equalsIgnoreCase("*")) {
                result += "0288.wav" + ":";
            }
            
            if(l.equalsIgnoreCase("#")) {
                result += "0289.wav" + ":";
            }
        }
        
        if (result.contentEquals("")) {
            return "";
        }

        // remove trailing ":"
        result = result.substring(0, result.length() - 1);
        return appendPrefix(result);
    }

    /**
     * Speak list of prompts (for variable inserted prompts)
     */
    @Override
    public String prompts() {
        // NB No append prefix here.
        return getValue();
    }

    // speak 0..9 as "Oh", "Oh One", "Oh Two" etc.
    @Override
    protected String ohJoy(int num) {

        if(num < 10) {
            return String.format("%04d.wav", num + 205);
        }

        return "";
    }

    @Override
    protected String year(Date d) {
        
        // CallPilot does not announce year
        return "";
    }

    @Override
    protected String month(Date d) {
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(d);
        int month = calendar.get(Calendar.MONTH);
        
        return String.format("%04d.wav", month + 148);
    }

    @Override
    protected String dayOfMonth(Date d) {
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(d);
        int day = calendar.get(Calendar.DAY_OF_MONTH);
        return smallOrdinal(day);
    }

    @Override
    protected String dayOfWeek(Date d) {
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(d);
        int day = calendar.get(Calendar.DAY_OF_WEEK);        
        return String.format("%04d.wav", day + 706);
    }

    @Override
    protected String time24(Date d) {
        
        String result = "";
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(d);
        int hour = calendar.get(Calendar.HOUR_OF_DAY);
        int min = calendar.get(Calendar.MINUTE);

        if (hour == 0 && min == 0) {
            // "Zero Hundred Hours" but we don't have "hundred.wav",
            // So say "Zero hours" for now.
            return "0.wav:hours.wav";
        }
        if (hour == 0) {
            // oh oh
            result = "o.wav:o.wav";
        } else if (hour < 10) {
            // Oh one hundred
            result = "o.wav:"+ smallCardinal(hour*100) ;
        } else {
            result = smallCardinal(hour);
        }
        
        result += ":" + String.format("%04d.wav", min + 204);

        return result;
    }

    @Override
    protected String time12(Date d) {
        String result = "";
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(d);
        int hour = calendar.get(Calendar.HOUR_OF_DAY);
        int min = calendar.get(Calendar.MINUTE);
        String meridian = hour < 12 ? ":0264.wav" : ":0265.wav";

        if (hour == 0 && min == 0) {
            // Twelve midnight
            return "0267.wav";
        }
        if (hour == 12 && min == 0) {
            // Twelve noon
            return "0266.wav";
        }
        
        if (hour > 12) {
            hour -= 12; 
        }

        result = String.format("%04d.wav", hour + 192);
        result += ":" + String.format("%04d.wav", min + 204);

        // Don't forget "AM" or "PM"
        result += meridian;
        return result;
    }
}
