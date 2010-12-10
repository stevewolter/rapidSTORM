
// SimParm: Simple and flexible C++ configuration framework
// Copyright (C) 2007 Australian National University
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 
// Contact:
// Kevin Pulo
// kevin.pulo@anu.edu.au
// Leonard Huxley Bldg 56
// Australian National University, ACT, 0200, Australia

//package simparm.twiddler;

import java.io.*;
import java.text.ParseException;
import java.text.NumberFormat;

class ConfigEntryDouble extends ConfigEntryNumber<Double> {

	public ConfigEntryDouble() { 
            super(); 
            setValue(new Double(0), false, true);
        }

        public Double parseSimparmValue(String value)
            throws ParseException 
        {
            double val = 0;
            if (value.equalsIgnoreCase("inf") 
                || value.equalsIgnoreCase("infinity"))
                val = Double.POSITIVE_INFINITY;
            else if (value.equalsIgnoreCase("-inf"))
                val = Double.NEGATIVE_INFINITY;
            else
                val = Double.parseDouble(value);
            return new Double(val);
        }

        public Double parseGUIValue(String value)
            throws ParseException 
        {
            double val = 0;
            if (value.equalsIgnoreCase("inf") 
                || value.equalsIgnoreCase("infinity"))
                val = Double.POSITIVE_INFINITY;
            else if (value.equalsIgnoreCase("-inf"))
                val = Double.NEGATIVE_INFINITY;
            else
                val = 
                    NumberFormat.getInstance().parse(value).doubleValue();
            return new Double(val);
        }


        public String printGUIValue(Double value) {
            return 
                NumberFormat.getInstance().format( value.doubleValue() );
        }

        public String printSimparmValue(Double value) {
            return value.toString();
        }

        public int getPercentageValue() {
            if (maxvalue == minvalue) return 0;
            int p = (int)((value - minvalue)
                * 100 / (maxvalue-minvalue));
            return p;
        }
        public void setPercentageValue(int percentage) {
            double nv = percentage*
                (maxvalue-minvalue) / 100 + minvalue;
            setValue(new Double(nv), true, true);
        }

        public void adjustValue(int amount) {
		setValue(new Double(value - amount * increment),
                         true, true);
	}
}

