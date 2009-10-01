#ifndef FOREACH_H
#define FOREACH_H

#define foreach(index, type, list) \
   for(type::iterator (index) = (list).begin(); index != (list).end(); index++)
#define foreach_const(index, type, list) \
   for(type::const_iterator index = (list).begin(); index != (list).end(); index++)

#endif
