package au.com.pulo.kev.simparm;

interface NumberParser {
    public String set_value_from_simparm( String simparm_value, int index );
    public boolean has_percentage();
    public int get_percentage();

    public void set_string_value( String value );
    public void increment_value(int rotations);
    public void set_percentage(int percents);

    public String get_simparm_value( int index );
}
