#ifndef SIMPARM_LINK_HH
#define SIMPARM_LINK_HH

#include <list>
#include <iterator>
#include <memory>

namespace simparm {

template <class Type, class List> 
struct LinkEnd 
{
  private:
    Type& object;
    List& list;
    typename List::iterator iter;

    LinkEnd( const LinkEnd& );
    LinkEnd& operator=( const LinkEnd& );

  public:
    LinkEnd(Type& object, List& list, typename List::value_type list_node,
            typename List::iterator where);
    ~LinkEnd();

    const Type* operator->() const { return &object; }
    const Type& get() const { return object; }
    Type* operator->() { return &object; }
    Type& get() { return object; }
    const typename List::iterator& getIterator() const { return iter; }
};

template <class From, class To>
class Link {
    template <class BaseType, bool up> class _iterator;
    template <class BaseType, bool up> class _const_iterator;

  public:
    struct List : public std::list<Link*> { ~List(); };

    typedef _iterator<From, true> up_iterator;
    typedef _const_iterator<From, true> const_up_iterator;
    typedef _iterator<To, false> down_iterator;
    typedef _const_iterator<To, false> const_down_iterator;

    enum WhichEnd { FirstEnd, SecondEnd };

  private:
    std::auto_ptr<From> up_destroyer;
    std::auto_ptr<To> down_destroyer;
    LinkEnd< From, std::list<Link*> > f;
    LinkEnd< To, std::list<Link*> > t;

    Link( const Link& );
    Link& operator=( const Link& );

  public:

    Link( From& up, To& down );
    Link( From& up, To& down, 
          const down_iterator& up_where,
          const up_iterator& down_where );
    ~Link();

    const From& up_end() const { return f.get(); }
    const To& down_end() const { return t.get(); }
    From& up_end() { return f.get(); }
    To& down_end() { return t.get(); }

    void delete_up_end_on_link_break()
        { up_destroyer.reset( &f.get() ); }
    void delete_down_end_on_link_break()
        { down_destroyer.reset( &t.get() ); }
    bool deletes_up_end() const { return up_destroyer.get() != NULL; }
    bool deletes_down_end() const { return down_destroyer.get() != NULL; }

    const_down_iterator get_iterator_in_up() const
        { return f.getIterator(); }
    const_up_iterator get_iterator_in_down() const
        { return t.getIterator(); }
    down_iterator get_iterator_in_up() 
        { return f.getIterator(); }
    up_iterator get_iterator_in_down()
        { return t.getIterator(); }
};

}

#endif
