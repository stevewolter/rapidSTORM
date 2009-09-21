#ifndef SIMPARM_NODE
#define SIMPARM_NODE

#include <string>
#include <list>
#include <stdexcept>
#include <memory>

namespace simparm {

class Node {
  private:
    bool subtree_is_dag() const;

  public:
    class Callback : public std::unary_function<Node&,void> {
        struct Data;
        Data *data;
        friend class Node;

      public:
        enum Cause { AddedChild, AddedParent, RemovedChild, RemovedParent, 
                     ValueChanged, ActivityChanged, Other };

        Callback();
        Callback(const Callback&);
        virtual ~Callback();
        Callback& operator=(const Callback&);

        void receive_changes_from(Node &node)
            { node.addChangeCallback(*this); }

        /** Event receival method. */
        virtual void operator()(Node& source_of_event,
                                Cause cause,
                                Node* argument_if_any) = 0;
    };

    typedef std::list<Node*>::iterator iterator;
    typedef std::list<Node*>::const_iterator const_iterator;

  private:
    /* Encapsulate private data to reduce number of needed inclusions. */
    struct Data;
    Data* data;
    std::list<Node*> parents, children;
    std::list<Callback*> changecallbacks;

    int _active;
    bool declared;
    std::string *_currentlyDefining;

    /* Deferred callback management - needed when callback signals its own
     * removal. The variable \c currently_iterating_changecallbacks counts
     * the number of accesses to the changecallbacks list. Every loop iterating
     * this vector increases the variable, and any call to addChangeCallback or
     * removeChangeCallback is diverted into the \c deferred_callback_actions
     * member of the \c data structure. When the outermost loop terminates, the
     * delayed callback additions and removals are performed. */
    int currently_iterating_changecallbacks;
    enum CallbackOp { Add, Remove };
    void do_deferred_callback_management();
    void manage_callback( CallbackOp action, Callback *argument );

    void add_object( Node& object, bool front );
    
  protected:
    void notifyChangeCallbacks( Callback::Cause cause, Node *argument_if_any );

    void removeFromAllParents();
    void removeAllChildren();

    virtual std::string getTypeDescriptor() const
 = 0;

  public:
    Node();
    Node(const Node& o);
    virtual ~Node();
    virtual Node *clone() const = 0;
    Node& operator=(const Node&o);

    /** Supplies a name for this node. Every method which overrides this
     *  method must also make sure to call removeFromAllParents() before
     *  its' destructor is finished since the name is needed in
     *  destruction. */
    virtual std::string getName() const = 0;
    virtual std::string getPrefix() const { return "in"; }

    virtual void push_front(Node& object);
    virtual void push_back(Node& object);

    virtual void erase(Node& object);
    /** @throw invalid_argument when no Node with the given name is 
     *         found amongst the direct children. */
    virtual void erase(const std::string& name) 
;
    virtual void clear();

    void make_to_sibling_of(const Node &other);

    /** Legacy interface for add() */
    virtual void register_entry(Node* object)
        { if (object != NULL) push_back(*object); }

    virtual bool isActive() { return _active != 0; }
    virtual void setActivity(bool active);

    virtual void print(const std::string& what);

    virtual std::string define();
    virtual std::string undefine();
    virtual std::list<std::string> printValues() const;
    virtual void printHelp(std::ostream &) const {}
    virtual void processCommand(std::istream& from) 
;

    void addChangeCallback(Callback& callback);
    void removeChangeCallback(Callback& callback);

    const std::list<Node*>& getChildren() const 
        { return children; }
    const std::list<Node*>& getParents() const 
        { return parents; }

    /** Destruct o when this object is destructed.*/
    void manage(Node* o); 
    /** Destruct the content of o when this object is destructed.*/
    void manage(std::auto_ptr<Node> o) 
        { this->manage(o.release()); }

    const Node& operator[](const std::string& name) const
;
    Node& operator[](const std::string& name)
;
    int size() const { return children.size(); }
    std::list<Node*>::const_iterator begin() const
        { return children.begin(); }
    std::list<Node*>::const_iterator end() const
        { return children.end(); }

    bool has_child_named(const std::string& name) const;
};

}

#endif
