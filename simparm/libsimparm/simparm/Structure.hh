#ifndef SIMPARM_STRUCTURE_HH
#define SIMPARM_STRUCTURE_HH

namespace simparm {

template <typename BaseType> 
class VirtualStructure 
: public BaseType 
{
  public:
    VirtualStructure()
        { this->BaseType::registerNamedEntries(); }
    VirtualStructure(const VirtualStructure<BaseType>& copy_from) 
        : Node(copy_from), BaseType(copy_from)
        { this->BaseType::registerNamedEntries(); }
    VirtualStructure(const BaseType& copy_from) 
        : Node(copy_from), BaseType(copy_from)
        { this->BaseType::registerNamedEntries(); }
};

template <typename BaseType> 
class Structure 
: public VirtualStructure<BaseType>
{
  public:
    Structure() {}
    Structure(const Structure<BaseType>& copy_from) 
        : Node(copy_from), VirtualStructure<BaseType>(copy_from) {}
    Structure(const VirtualStructure<BaseType>& copy_from) 
        : Node(copy_from), VirtualStructure<BaseType>(copy_from) {}
    Structure(const BaseType& copy_from) 
        : Node(copy_from), VirtualStructure<BaseType>(copy_from) {}

    virtual Structure<BaseType>* clone() const
        { return new Structure<BaseType>(*this); }
};

}
#endif
