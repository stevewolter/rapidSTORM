#include <wx/wx.h>
#include <iostream>

const wxChar *SI_prefixes[] = {
    wxT("f"), wxT("p"), wxT("n"), wxT("mu"), wxT("m"),
                    wxT(""),
    wxT("k"), wxT("M"), wxT("G"), wxT("T"), wxT("E") };

void make_SI_prefix( float original_value, float& rest, const wxChar *& unit_prefix )
{
    if ( fabs(original_value) <= 1E-18 ) {
        rest = 0;
        unit_prefix = _T("");
    } else {
        float abs_val = fabs(original_value);
        int postfix = int(floor( log10( abs_val ) / 3 ));
        postfix = std::min( std::max( -5, postfix ), 5 );
        rest = original_value / pow( 1000, postfix );
        unit_prefix = SI_prefixes[postfix+5];
    }
}

std::ostream& operator<<(std::ostream& o, const wxRect& rect ) {
    return (o <<  "(" << rect.GetLeft() <<  ", " << rect.GetTop() << ")+"
              << "(" << rect.GetRight() << ", " << rect.GetBottom() << ")");
}
