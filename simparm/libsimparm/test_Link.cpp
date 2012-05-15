#include <dejagnu.h>
#include "Link.hh"
#include "Link_impl.hh"
#include "Link_iterator.hh"
#include <iostream>
#include <cassert>

struct Phony {
    typedef simparm::Link<Phony,Phony> Link;

    Link::List from, to;
    std::string name;
    bool destroyed;

    Phony(std::string n) : name(n), destroyed(false) {}
    ~Phony() { 
        assert( !destroyed );
        destroyed = true;
    }

    void link_removed( Link::WhichEnd, Phony & ) {
    }

    template <class A, class B, bool>
        Link::List& get_link_list();
};

template<>
Phony::Link::List& 
Phony::get_link_list<Phony,Phony,true>() { return from; }
template<>
Phony::Link::List& 
Phony::get_link_list<Phony,Phony,false>() { return to; }

void print_tree( const Phony& p, std::string prefix ) {
    std::cerr << prefix << " " << p.name << "\n";
    for ( Phony::Link::const_down_iterator i = p.to.begin();
                                           i != p.to.end() ; i++)
    {
        print_tree( *i, prefix + "    " );
    }
}

int main() {
    Phony a("a"), b("b"), c("c");
    {
        Phony d("d"), e("g"), f("f");
        new Phony::Link(a, e);
        new Phony::Link(e, b);
        new Phony::Link(b, c);
        new Phony::Link(b, d);
        (new Phony::Link(d, *(new Phony("e"))))
            ->delete_down_end_on_link_break();
        new Phony::Link(a, f);

        //print_tree(a, " ");
    }
    //print_tree(a, " ");
    //print_tree(b, " ");
    pass("Linking back and forth works");
}
