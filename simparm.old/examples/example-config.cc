
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

#include "Set.hh"

using namespace std;

namespace Convergence {
  enum {
    nothing,
    exit_program,
    reduce_timestep
  };
}

class SetApplication : public Set {
	public:
		EntryDouble timestep;
		EntryEntry<unsigned long> max_t;
		EntryDouble epsilon;
	  EntryBool running;
	  EntryBool finished;
	  EntryBool autorestart;
  EntryString input_filename;
  EntryString output_filename;
ChoiceEntry convergence;

EntryDivider divider1;
EntryDivider divider2;
EntryDivider divider3;
EntryDivider divider4;
EntryTrigger yield;
		EntryDouble error;
		EntryDouble improvement;
		EntryDouble improvement_percent;

		SetApplication();
};


SetApplication::SetApplication()
: Set()
{
  timestep.setName("relax.timestep");
  timestep.setDesc("Relax timestep");
  timestep = 1.0;
  timestep.setIncrement(0.1);
  timestep.setMin(0.0);
  register_entry(&timestep);

  max_t.setName("max_t");
  max_t.setDesc("Relax max timesteps");
  max_t = 1000000;
  max_t.setIncrement(10000);
  max_t.setMin(0);
  register_entry(&max_t);

  epsilon.setName("epsilon");
  epsilon.setDesc("Relax epsilon");
  epsilon = 0.1;
  epsilon.setIncrement(0.01);
  epsilon.setMin(0.0);
  register_entry(&epsilon);

  register_entry(&divider1);

  running.setName("running");
  running.setDesc("Running");
  running = true;
  register_entry(&running);

  finished.setName("finished");
  finished.setDesc("Finished");
  finished = false;
  register_entry(&finished);

  autorestart.setName("autorestart");
  autorestart.setDesc("Auto-restart");
  autorestart = true;
  register_entry(&autorestart);

  register_entry(&divider2);

  input_filename.setName("input_filename");
  input_filename.setDesc("Input filename");
  register_entry(&input_filename);

  output_filename.setName("output_filename");
  output_filename.setDesc("Output filename");
  register_entry(&output_filename);

  register_entry(&divider3);

convergence.setName("convergence");
convergence.setDesc("Convergence action");
convergence.addChoice(Convergence::
    nothing, "nothing", "Nothing");
convergence.addChoice(Convergence::
    exit_program, "exit_program",
    "Exit program");
convergence.addChoice(Convergence::
    reduce_timestep, "reduce_timestep",
    "Reduce timestep");
convergence = Convergence::nothing;
register_entry(&convergence);

  yield.setName("yield");
  yield.setDesc("Yield");
  register_entry(&yield);

  register_entry(&divider4);

  error.setName("error");
  error.setDesc("Error");
  error = 0.325226;
  error.setEditable(false);
  register_entry(&error);

  improvement.setName("improvement");
  improvement.setDesc("Error improvement");
  improvement = 0.125023;
  improvement.setEditable(false);
  register_entry(&improvement);

  improvement_percent.setName("improvement_percent");
  improvement_percent.setDesc("Error improvement (%)");
  improvement_percent = 61.558116;
  improvement_percent.setEditable(false);
  register_entry(&improvement_percent);

}



#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <cmath>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <glob.h>
#include <ext/stdio_filebuf.h>

using namespace std;


static SetApplication config;


int main(int , char *[]) {

	cerr << config;

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	config.setIO(&cin, fileno(stdin), &cout, fileno(stdout));
	while (true) {
		config.serviceInput();
		usleep(10000);
	}

	return 0;
}

