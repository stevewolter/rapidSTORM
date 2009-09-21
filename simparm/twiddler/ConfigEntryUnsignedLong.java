
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


class ConfigEntryUnsignedLong extends ConfigEntryNumber<Long> {
	public ConfigEntryUnsignedLong() { 
            super();
            setValue(new Long(0), false, true, true);
        }

        public Long parseGUIValue(String value)
            throws NumberFormatException 
        {
            long val = 0;
            val = Long.parseLong(value);
            return new Long(val);
        }

        public Long parseSimparmValue(String value)
            throws NumberFormatException 
        {
            long val = 0;
            val = Long.parseLong(value);
            return new Long(val);
        }

        public String printGUIValue(Long value) {
            return value.toString();
        }

        public String printSimparmValue(Long value) {
            return value.toString();
        }

        public Long modifyByIncrements(Long x, int i) {
		return (new Long(x - i * increment));
	}

        public Long slider_pair_to_value(SliderPair pair) {
            if ( maxvalue == null || minvalue == null )
                return value;
            double range = maxvalue.doubleValue() - minvalue.doubleValue();
            if ( range <= 0 ) return maxvalue;
            return new Long((long)( pair.major * range / 100.0
                                    + minvalue.doubleValue() ));
        }
}

