
package au.com.pulo.kev.simparm;

import java.math.BigInteger;

class ConfigEntryInteger extends ConfigEntryNumber {
    static class Parser implements NumberParser {
        /* We have to use integer here since 64 bit unsigned long cannot be represented in Java */
        BigInteger[] values = new BigInteger[4];

        public String get_simparm_value( int index ) 
            { return values[index].toString(); }
        public String set_value_from_simparm( String value, int index ) 
            { values[index] = new BigInteger(value); if ( values[0] != null ) return values[0].toString(); else return null; }
        public void set_string_value( String value )
            { values[0] = new BigInteger(value); }
        public void increment_value(int rotations) {
            if ( values[NumberField.Value] == null || values[NumberField.Increment] == null ) 
                return;
            BigInteger factor = new BigInteger( Integer.toString(rotations) );
            values[NumberField.Value] = values[NumberField.Value].add( factor.multiply(values[NumberField.Increment]) );
        }
        public boolean has_percentage() {
            return values[NumberField.Min] != null && values[NumberField.Max] != null;
        }
        public int get_percentage() {
            return (values[NumberField.Value].subtract(values[NumberField.Min])).multiply(new BigInteger("100")).divide( 
                values[NumberField.Max].subtract(values[NumberField.Min]) ).intValue();
        }
        public void set_percentage(int percents) {
            values[NumberField.Value] = values[NumberField.Max].subtract(values[NumberField.Min]).multiply( 
                new BigInteger( Integer.toString(percents) ) ).divide( new BigInteger("100") ).add( values[NumberField.Min] );
        }
    }
    public NumberParser make_parser() {
        return new Parser();
    }

}
