#ifndef DSTORM_OUTPUT_BINNING_CONFIG_H
#define DSTORM_OUTPUT_BINNING_CONFIG_H

#include <simparm/Node.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ObjectChoice.hh>
#include <simparm/ManagedChoiceEntry.hh>
#include <memory>
#include <boost/ptr_container/ptr_vector.hpp>
#include "binning_decl.h"
#include <dStorm/localization/Traits.h>
#include <dStorm/make_clone_allocator.hpp>

namespace dStorm {
namespace output {
namespace binning {

struct FieldConfig : public simparm::ObjectChoice {
    FieldConfig( std::string name, std::string desc ) : simparm::ObjectChoice(name,desc) {}
    virtual FieldConfig* clone() const = 0;

    virtual std::auto_ptr<Scaled> make_scaled_binner() const = 0;
    virtual std::auto_ptr<Unscaled> make_unscaled_binner() const = 0;
    virtual std::auto_ptr<UserScaled> make_user_scaled_binner() const = 0;
    virtual void set_visibility(const input::Traits<Localization>&, bool unscaled_suffices) = 0;

    virtual void add_listener( simparm::BaseAttribute::Listener& l ) = 0;
};

struct FieldChoice 
: public simparm::ManagedChoiceEntry<FieldConfig>
{
  private:
    template <int Field> void fill(BinningType type, std::string axis);
    
  public:
    FieldChoice( const std::string& name, const std::string& desc, BinningType type, std::string axis_spec );
    FieldChoice( const FieldChoice& );
    FieldChoice& operator=( const FieldChoice& );
    void set_visibility(const input::Traits<Localization>&, bool unscaled_suffices);

    FieldChoice* clone() const { return new FieldChoice(*this); }
    ~FieldChoice();
    void add_listener( simparm::BaseAttribute::Listener& l );
};

}
}
}

DSTORM_MAKE_BOOST_CLONE_ALLOCATOR(dStorm::output::binning::FieldConfig)
DSTORM_MAKE_BOOST_CLONE_ALLOCATOR(dStorm::output::binning::FieldChoice)


#endif
