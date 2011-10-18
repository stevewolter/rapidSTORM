#ifndef DSTORM_INPUT_CHAIN_ENGINE_HELPERS
#define DSTORM_INPUT_CHAIN_ENGINE_HELPERS

namespace dStorm {
namespace input {
namespace chain {

struct ContextTraitCreator {
    input::chain::Context& c;
    ContextTraitCreator(input::chain::Context& c) : c(c) {}

    template <typename Type>
    void operator()( const Type& ) {
        c.more_infos.push_back( new input::Traits<Type>() );
    }
    void operator()( const output::LocalizedImage& ) {
        c.more_infos.push_back( new input::Traits<output::LocalizedImage>("Engine", "Engine context" ) );
    }
};

}
}
}

#endif
