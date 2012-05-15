#ifndef SIMPARM_CALLBACK
#define SIMPARM_CALLBACK

#include <memory>
#include <functional>
#include <cstddef>

namespace simparm {

class Node;
class Listener;
class Publisher;

struct Event {
    enum Cause { 
        AddedChild,
        AddedParent,
        ValueChanged,
        ActivityChanged,
        Other,
        All };

    const Publisher& source;
    Cause cause;

    Event( const Publisher& s, Cause c )
        : source( s ), cause( c ) {}
};

struct LinkChangeEvent : public Event
{
  public:
    Publisher& other;

    LinkChangeEvent( 
        const Publisher& s, Cause c,
        Publisher& n )
        : Event( s, c ), other(n) {}
};

class Publisher {
    class Pimpl;
    std::auto_ptr<Pimpl> data;

    friend class Listener;

  protected:
    void notifyChangeCallbacks( Event::Cause cause, Publisher *argument_if_any = NULL );

  public:
    Publisher();
    ~Publisher();

    void addChangeCallback(Listener& callback);
    void removeChangeCallback(Listener& callback);
};

class Listener 
: public std::unary_function<Node&,void> {
    struct Data;
    Data *data;

    friend class Publisher;
    friend class Publisher::Pimpl;

  public:
    explicit Listener( Event::Cause limit_to = Event::All );
    Listener(const Listener&);
    virtual ~Listener();
    Listener& operator=(const Listener&);

    void receive_changes_from(Publisher &node)
        { node.addChangeCallback(*this); }
    void stop_receiving_changes_from(Publisher &node)
        { node.removeChangeCallback(*this); }

    /** Event receival method. */
    virtual void operator()(const Event&) = 0;
};

}

#endif
