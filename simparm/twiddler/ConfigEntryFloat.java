package au.com.pulo.kev.simparm;

import java.text.NumberFormat;
import java.text.DecimalFormat;
import java.text.ParseException;

class ConfigEntryFloat extends ConfigEntryNumber {
    static class FloatParser implements NumberParser {
        Double[] values = new Double[4];
        NumberFormat format = NumberFormat.getInstance();
        DecimalFormat scientific_format = new DecimalFormat("0.0##E0");

        public String get_simparm_value( int index ) 
            { return ( values[index] != null ) ? values[index].toString() : null; }
        public void set_string_value( String value ) {
            double val = 0;
            if (value.equalsIgnoreCase("inf") 
                || value.equalsIgnoreCase("infinity"))
                val = Double.POSITIVE_INFINITY;
            else if (value.equalsIgnoreCase("-inf"))
                val = Double.NEGATIVE_INFINITY;
            else {
                try {
                    val = format.parse(value.replace('e', 'E')).doubleValue();
                } catch ( java.text.ParseException e ) {
                    System.err.println("Invalid number in simparm string: " + value);
                } finally {}
            }
            values[0] = new Double(val);
        }
        public String set_value_from_simparm( String value, int index ) {
            double val = 0;
            if (value.equalsIgnoreCase("inf") 
                || value.equalsIgnoreCase("infinity"))
                val = Double.POSITIVE_INFINITY;
            else if (value.equalsIgnoreCase("-inf"))
                val = Double.NEGATIVE_INFINITY;
            else
                val = Double.parseDouble(value);
            values[index] = new Double(val);
            if ( values[0] == null )
                return null;
            else {
                double abs = Math.abs(val);
                if ( abs == 0 || (abs > 1E-3 && abs < 1E4 ) ) 
                    return format.format(val);
                else
                    return scientific_format.format(val);
            }
        }
        public void increment_value(int rotations) {
            /* TODO: Implement */
        }
        public boolean has_percentage() {
            return values[NumberField.Value] != null && values[NumberField.Min] != null && values[NumberField.Max] != null;
        }
        public int get_percentage() {
            return (int)Math.round( 
                (values[NumberField.Value] - values[NumberField.Min]) * 100
                    / (values[NumberField.Max] - values[NumberField.Min]) );
        }
        public void set_percentage(int percents) {
            values[NumberField.Value] = (values[NumberField.Max] - values[NumberField.Min]) * percents / 100
                + values[NumberField.Min];
        }
    }

    public NumberParser make_parser() {
        return new FloatParser();
    }
}
