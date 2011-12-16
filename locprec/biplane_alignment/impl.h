#ifndef LOCPREC_BIPLANE_ALIGNMENT_H
#define LOCPREC_BIPLANE_ALIGNMENT_H

#include <dStorm/Image.h>
#include <dStorm/engine/Image.h>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/log.h>
#include <simparm/Entry.hh>
#include <simparm/Object.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Structure.hh>
#include <boost/iterator/iterator_adaptor.hpp>
#include <dStorm/input/chain/Filter.h>

namespace locprec {
namespace biplane_alignment {

struct MotionModel {
    typedef Eigen::Matrix<double, 2, Eigen::Dynamic> FieldJacobian;
    typedef Eigen::Matrix3d Motion;

    struct IsIdentity {};

    virtual ~MotionModel() {}
    virtual MotionModel* clone() const = 0;
    virtual simparm::Node& getNode() = 0;
    virtual Motion get_motion( const Eigen::Matrix<double, Eigen::Dynamic, 1>& variables ) const = 0;
    virtual bool is_identity() const { return false; }
    virtual int parameter_count() const = 0;

    operator simparm::Node&() { return getNode(); }
    operator const simparm::Node&() const { return const_cast<MotionModel&>(*this).getNode(); }
};

struct Config : public simparm::Object {
    typedef boost::mpl::vector< dStorm::engine::Image > SupportedTypes;
    simparm::NodeChoiceEntry<MotionModel> model;

    Config();
    Config* clone() const { return new Config(*this); }
    void registerNamedEntries() { push_back(model); }
};

struct Source
: public simparm::Object,
  public dStorm::input::AdapterSource< dStorm::engine::Image >
{
    typedef dStorm::input::Source<dStorm::engine::Image> Base;

    Source( const Config& c, std::auto_ptr< Base > base);
    Base::iterator begin();
    Base::iterator end();
    void modify_traits( dStorm::input::Traits< dStorm::engine::Image >&);
    
  private:
    friend class Derivator;
    typedef dStorm::Image< float, 3 > Image;
    typedef dStorm::Image< float, 2 > Plane;
    typedef dStorm::Image< float, 1 > Line;
    std::auto_ptr< MotionModel > model;
    MotionModel::Motion motion;
    class _iterator;

    typedef std::vector<dStorm::engine::Image> Images;

    struct Whitening;

    template <typename Pixel>
    static void apply_motion( const dStorm::Image<Pixel,2>&, const MotionModel::Motion&, dStorm::Image<Pixel,2>& );
    Whitening get_whitening_factors();

    simparm::Node& node() { return *this; }
};

struct Filter
: public dStorm::input::chain::Filter
{
  public:
    typedef dStorm::input::chain::DefaultVisitor<Config> Visitor;
    simparm::Structure<Config> config;
    const Config& get_config() { return config; }

    Filter() : dStorm::input::chain::Filter() {}
    ~Filter() {}
    Filter* clone() const { return new Filter(*this); }

    AtEnd traits_changed( TraitsRef, Link* );
    AtEnd context_changed( ContextRef, Link* );
    dStorm::input::BaseSource* makeSource();

    simparm::Node& getNode() { return static_cast<Config&>(config); }
};

}
}

#endif
