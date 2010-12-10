/** \file ControlStateListener.cpp
 *  This file contains the definitions for the ControlStateListener */
#include <AndorCamera/ControlStateListener.h>
#include <set>

#include <cc++/thread.h>
#include <iostream>

using namespace std;
using namespace AndorCamera;

/** Set to store the locations of the ControlStateListener classes.
 *  This variable may be static since Control is a singleton. */
static int usageCounter = 0;
static std::set<ControlStateListener*>* l = NULL;

/* See AndorCamera/ControlStateListener.h for documentation. */
void ControlStateListener::notify(State newState) throw(std::exception) { 
    PROGRESS("Notifying listeners of control state change");
    set<ControlStateListener*>::iterator i;
    for (i = l->begin(); i != l->end(); i++) {
        (*i)->controlStateChanged(newState);
    }
    PROGRESS("Notified listeners of control state change");
}

/* See AndorCamera/ControlStateListener.h for documentation. */
ControlStateListener::ControlStateListener() throw() { 
    PROGRESS("Constructor entry");
    LOCKING("Set pointer is " << l);
    if (l == NULL)
        l = new std::set<ControlStateListener*>();
    usageCounter++;
    LOCKING("Inserting; set pointer is " << l);
    l->insert(this); 
    LOCKING("Constructor exit");
}
/* See AndorCamera/ControlStateListener.h for documentation. */
ControlStateListener::~ControlStateListener() throw() { 
    l->erase(this); 
    usageCounter--;
    if (usageCounter == 0) {
        delete l;
        l = NULL;
    }
}
