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
public class TextToPrompts_en extends TextToPrompts {

    /**
     * Convert cardinal numbers 1000 or less
     *
     * @param card
     * @return
     */
    protected String smallCardinal(int card) {
        String result = "";
        if (card > 100) {
            int hundreds = card / 100;
            result = String.format("%d00.wav", hundreds);
            card -= hundreds * 100;
            if (card == 0) {
                return result;
            }
            result += ":";
        }
        if (card > 0) {
            return result + String.format("%02d.wav", card);
        }
        return "0.wav";
    }

    /**
     * Convert positive cardinal numbers up to 999,999
     *
     * @param card
     * @return
     */
    protected String largeCardinal(int card) {
        String result = "";

        if (card > 1000000) {
            // This prompt set doesn't have million, billion, etc. Just use digits
            return digits();
        }

        // Special Case, 2002 thru 2020
        if (card >= 2002 && card <= 2020) {
            return String.format("%d.wav", card);
        }

        // 999,999 thru 1000
        if (card >= 1000) {
            int thousands = card / 1000;
            card -= thousands * 1000;

            if (thousands > 9) {
                result = smallCardinal(thousands) + ":thousand.wav";
            } else {
                // Exact thousand. Use 1000.wav, 2000.wav, etc.
                result = String.format("%d000.wav", thousands);
            }
            if (card == 0) {
                return result;
            }
            result += ":";
        }

        // Values from 0-999 are easy!
        return result + smallCardinal(card);
    }

    /**
     * Convert a small Ordinal (small is 31 or less!
     * There are prompts for 40th, and 50th, however.)
     *
     * @param ord
     * @return
     */
    protected String smallOrdinal(int ord) {
        // Special cases
        switch (ord) {
        case 1:
            return "1st.wav";
        case 2:
            return "2nd.wav";
        case 3:
            return "3rd.wav";
        case 21:
            return "21st.wav";
        case 22:
            return "22nd.wav";
        case 23:
            return "23rd.wav";
        case 31:
            return "31st.wav";
        case 40:
            return "40th.wav";
        case 50:
            return "50th.wav";
        default:
        }

        // Easy to calculate
        if (ord < 31) {
            return String.format("%dth.wav", ord);
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
            // TODO Currently no way of saying minus or negative with current prompt set
            // result = "minus.wav:" ;
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

        // Cannot handle ordinals greater than 50 or less than 0, or even 0!
        if (origValue <= 0 || origValue > 50) {
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
        for (int i = 0; i < v.length(); i++) {
            String l = v.substring(i, i + 1);
            if ("abcdefghijklmnopqrstuvwxyz".contains(l)) {
                result += l + ".wav:";
            } else if ("0123456789".contains(l)) {
                result += smallCardinal(Integer.parseInt(l)) + ":";
            } else {
                switch (l.charAt(0)) {
                case '@':
                    result += "at.wav:";
                    break;
                case ':':
                    result += "colon.wav:";
                    break;
                case '-':
                    result += "dash.wav:";
                    break;
                case '.':
                    result += "dot.wav:";
                    break;
                case '+':
                    result += "plus.wav:";
                    break;
                case '#':
                    result += "pound.wav:";
                    break;
                case '*':
                    result += "star.wav:";
                    break;
                default:
                    break;
                }
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
        switch (num) {
        case 0:
            return "o.wav:o.wav"; // oh oh
        case 1:
            return "o_one.wav";
        case 2:
            return "o_two.wav";
        case 3:
            return "o_three.wav";
        case 4:
            return "o_four.wav";
        case 5:
            return "o_five.wav";
        case 6:
            return "o_six.wav";
        case 7:
            return "o_seven.wav";
        case 8:
            return "o_eight.wav";
        case 9:
            return "o_nine.wav";
        default:
            return "";
        }
    }

    @Override
    protected String year(Date d) {
        String result = "";
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(d);
        int year = calendar.get(Calendar.YEAR);

        // Special Case, 2002 thru 2020
        if (year >= 2002 && year <= 2020) {
            result = String.format("%d.wav", year);
        } else {

            // Split into two parts, Century and decades so years can be spoken
            // as "Nineteen" (the century) "Twenty Nine" (the decade)
            int century = year / 100;
            int decade = year % 100;

            // Special case for millenium years
            if (century % 10 == 0) {
                // Render the Century part as "two thousand" or "three thousand"
                result = smallCardinal(century * 100);
                if (decade != 0) {
                    // Render the decade part as "one", "two", "twelve", etc.
                    result += ":" + smallCardinal(decade);
                }
            } else {
                // Render the Century part as "nineteen" or"twenty one"
                result = smallCardinal(century);
                // For the decade part, speak 01..09 as "Oh One", "Oh Two" etc.
                if (decade < 10) {
                    result += ":" + ohJoy(decade);
                } else {
                    // And 10-99 as a cardinal
                    result += ":" + smallCardinal(decade);
                }
            }

        }
        return result;
    }

    @Override
    protected String month(Date d) {
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(d);
        int month = calendar.get(Calendar.MONTH);

        switch (month) {
        case Calendar.JANUARY:
            return "january.wav";
        case Calendar.FEBRUARY:
            return "febrary.wav";
        case Calendar.MARCH:
            return "march.wav";
        case Calendar.APRIL:
            return "april.wav";
        case Calendar.MAY:
            return "may.wav";
        case Calendar.JUNE:
            return "june.wav";
        case Calendar.JULY:
            return "july.wav";
        case Calendar.AUGUST:
            return "august.wav";
        case Calendar.SEPTEMBER:
            return "september.wav";
        case Calendar.OCTOBER:
            return "october.wav";
        case Calendar.NOVEMBER:
            return "november.wav";
        case Calendar.DECEMBER:
            return "december.wav";
        default:
            return "";
        }
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
        switch (day) {
        case Calendar.SUNDAY:
            return "sunday.wav";
        case Calendar.MONDAY:
            return "monday.wav";
        case Calendar.TUESDAY:
            return "tuesday.wav";
        case Calendar.WEDNESDAY:
            return "wednesday.wav";
        case Calendar.THURSDAY:
            return "thursday.wav";
        case Calendar.FRIDAY:
            return "friday.wav";
        case Calendar.SATURDAY:
            return "saturday.wav";
        default:
            return "";
        }
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
        if (min == 0) {
            result += ":hours.wav";
        } else if (min < 10) {
            result += ":"+ ohJoy(min);
        } else {
            result += ":"+smallCardinal(min);
        }
        return result;
    }

    @Override
    protected String time12(Date d) {
        String result = "";
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(d);
        int hour = calendar.get(Calendar.HOUR_OF_DAY);
        int min = calendar.get(Calendar.MINUTE);
        String meridian = hour < 12 ? ":am.wav" : ":pm.wav";

        if (hour == 0 && min == 0) {
            // Twelve midnight
            return "12.wav:midnight.wav";
        }
        if (hour == 12 && min == 0) {
            // Twelve noon
            return "12.wav:noon.wav";
        }

        if (hour == 0) {
            hour = 12; // 00 is spoken 12
        } else if (hour > 12) {
            hour -= 12; // And 13..23 are 01..11
        }

        // 1..12 is HH
        result = smallCardinal(hour);

        if (min != 0) {
            if (min < 10) {
                // 01..09 is "HH: oh mm"
                result += ":" + ohJoy(min);
            } else {
                // 10..59 is "HH: mm"
                result += ":" + smallCardinal(min);
            }
        } else {
            // HH:00 is "HH o'Clock"
            result += ":o_clock.wav";
        }
        // Don't forget "AM" or "PM"
        result += meridian;
        return result;
    }
}
