#include "GUILabelTable.h"
#include "doc/guilabels.h"
#include <boost/tokenizer.hpp>

namespace simparm {

GUILabelTable::GUILabelTable() {
    for (size_t i = 0; i < sizeof(::guilabels) / sizeof(::guilabels[0]); ++i) {
        Entry entry;
        entry.description = guilabels[i][1];
        entry.helpID = guilabels[i][2];
        entries[guilabels[i][0]] = entry;
    }
}

GUILabelTable& GUILabelTable::get_singleton() {
    static GUILabelTable label_table;
    return label_table;
}

const GUILabelTable::Entry& GUILabelTable::get_entry( const std::string& name ) {
    return entries[name];
}

}
