#include "TransmissionConfig.h"
#include <list>

#include "EmissionTracker.h"
#include "LocalizationFile.h"
#include "Viewer.h"
#include "LocalizationCounter.h"
#include "ProgressMeter.h"
#include "Crankshaft.h"

#include "BasicTransmissions.h"

using namespace std;

template <typename T>
std::auto_ptr<T> make_auto_ptr(T* t) throw() 
    { return std::auto_ptr<T>(t); }

namespace dStorm {

    template <typename Type>
    class auto_list {
      private:
        std::list<Type*> base;

        void clear() throw() {
            for (typename std::list<Type*>::iterator i = base.begin();
                i != base.end(); i++)
            delete *i;
            base.clear();
        }
        void copy(const std::list<Type*>& from)
            throw() 
        {
            base.clear();
            std::transform( from.begin(), from.end(),
                back_inserter( base ), 
                mem_fun( &Type::clone ) );
        }

      public:
        auto_list() {}
        ~auto_list() { clear(); }

        auto_list(const auto_list<Type>& o) { copy(o.base); }
        auto_list& operator=(const auto_list<Type>& o) 
            { clear(); copy(o.base); }

        typedef typename std::list<Type*>::iterator iterator;
        typedef typename std::list<Type*>::const_iterator const_iterator;

        iterator begin() { return base.begin(); }
        const_iterator begin() const { return base.begin(); }
        iterator end() { return base.end(); }
        const_iterator end() const { return base.end(); }

        const Type& front() const throw() { return *base.front(); }
        const Type& back() const throw() { return *base.back(); }
        Type& front() throw() { return *base.front(); }
        Type& back() throw() { return *base.back(); }

        size_t size() const throw() { return base.size(); }

        void push_back(Type *p) throw() { base.push_back(p); }
    };


#if 0
static const char *nameDesc[][2] = {
    { "Table", "Write out fit coordinates" },
    { "Image", "Create result image" },
    { "Count", "Count found localizations" },
    { "Progress", "Show progress on file" },
    { "Trace", "Trace fluorophore activity" },
};
#endif

class TableOutputConfig : public TransmissionSource {
  private:
    ConfigEntryFile outputFile;

  public:
    TableOutputConfig() throw()
    : ConfigObject("Table", "Write out fit coordinates"),
      outputFile("ToFile", "Write localizations to")
    {
        outputFile.setHelp(
                        "If given, this parameters determines a file to which "
                        "the raw fit data will be written. The output is one "
                        "line per fit, with X- and Y-coordinate, image number "
                        "and fit amplitude, fields separated by spaces.");
        outputFile.setUserLevel(ConfigEntry::Beginner);
        push_back( outputFile );
    }
    TableOutputConfig(const TableOutputConfig& o) throw() 
    : ConfigObject(o), TransmissionSource(o),
      outputFile(o.outputFile)
    {
        push_back( outputFile );
    }

    ~TableOutputConfig() throw() {}

    TableOutputConfig* clone() const throw()
        { return new TableOutputConfig(*this); }
    std::auto_ptr<Transmission> makeTransmission() throw() {
        if ( outputFile )
            return std::auto_ptr<Transmission>
                ( new LocalizationFile(outputFile()) );
        else
            return std::auto_ptr<Transmission>( NULL );
    }
};

class ProgressChoice : public TransmissionSource {
  public:
    ProgressChoice() throw()
    : ConfigObject("Progress", "Display progress") {}
    ProgressChoice(const ProgressChoice& o) throw() 
    : ConfigObject(o), TransmissionSource(o) {}
    ~ProgressChoice() throw() {}

    ProgressChoice* clone() const throw()
        { return new ProgressChoice(*this); }
    std::auto_ptr<Transmission> makeTransmission() throw() {
        return std::auto_ptr<Transmission> ( new ProgressMeter() );
    }
};

class CountChoice : public TransmissionSource {
    public:
        CountChoice() throw()
        : ConfigObject("Count", "Count localizations") {}
        ~CountChoice() throw() {}

        CountChoice* clone() const throw() { return new CountChoice(*this); }
        std::auto_ptr<Transmission> makeTransmission() throw() {
            return std::auto_ptr<Transmission> ( new LocalizationCounter() );
        }

};

class ViewerModule : public TransmissionSource, public ViewerConfig {
  public:
    ViewerModule() throw()
    : ConfigObject("Image", "Result image") {}
    ~ViewerModule() throw() {}

    ViewerModule* clone() const throw()
        { return new ViewerModule(*this); }
    std::auto_ptr<Transmission> makeTransmission() throw() {
        if ( showOutput() || tifFile )
            return std::auto_ptr<Transmission> ( new Viewer(*this) );
        else
            return std::auto_ptr<Transmission>( NULL );
    }
};

template<typename Type>
void delete_arg(Type *p) { delete p; }

class CrankshaftModule 
: public TransmissionForwarderSource, public ConfigNode::Callback {
  private:
    auto_list<TransmissionSource> sources;
    int next_identity;

    void link_transmission( TransmissionSource& src ) throw() {
        std::stringstream nodeName;
        nodeName << "CrankshaftNode" << next_identity++;
        std::auto_ptr<ConfigObject> namedNode( 
            new GeneralConfigObject(nodeName.str(), "", "in", "ConfigObject") );
        namedNode->push_back( src );
        this->push_back( *namedNode );
        this->manage( namedNode.release() );
    }

    void addTransmissionSource( std::auto_ptr<TransmissionSource> src ) throw() {
        link_transmission( *src );
        sources.push_back( src.release() );
    }

    void add_default_transmission() throw() {
        std::auto_ptr<TransmissionSource> src = factory.makeTransmissionSource("");
        TransmissionChooser *lastEntry = 
            dynamic_cast<TransmissionChooser*>( src.get() );
        assert( lastEntry != NULL );
        lastEntry->set_auto_selection( false );
        lastEntry->setValue("");
        receive_changes_from( lastEntry->value );
        addTransmissionSource( src );
    }

    void initialize_forwarding_deferredly() throw() {
        add_default_transmission();
    }

  public:
    CrankshaftModule(TransmissionSourceFactory &tc) throw()
    : ConfigObject("Crankshaft", "Multiple outputs"),
      TransmissionForwarderSource(tc),
      next_identity(0)
    {
    }
    CrankshaftModule(const CrankshaftModule& o) throw() 
    : ConfigObject(o),
      TransmissionForwarderSource(o),
      ConfigNode::Callback(),
      sources(o.sources),
      next_identity(0)
    {
        for (auto_list<TransmissionSource>::iterator i = sources.begin(); 
             i != sources.end(); i++)
            link_transmission( **i );
        if ( sources.size() > 0 )
            receive_changes_from( sources.back() );
    }
    ~CrankshaftModule() throw() {}
    CrankshaftModule* clone() const throw() 
        { return new CrankshaftModule(*this); }

    std::auto_ptr<Transmission> makeTransmission() throw() {
        std::auto_ptr<Crankshaft> crankshaft( new Crankshaft() );

        std::list<TransmissionSource*>::iterator i;
        for( i = sources.begin(); i != sources.end(); i++ ) {
            crankshaft->add( (*i)->makeTransmission() );
        }

        if ( ! crankshaft->empty() )
            return std::auto_ptr<Transmission>(crankshaft);
        else
            return std::auto_ptr<Transmission>( NULL );
    }

    void operator()(ConfigNode &src, Cause cause, ConfigNode *) throw() {
        if (cause == ValueChanged) {
            src.removeChangeCallback( *this );
            add_default_transmission();
        }
    }
};

std::auto_ptr<TransmissionSource> BasicTransmissions::makeTransmissionSource
    (std::string wish) throw() 
{
    if ( wish == "Table" )
        return std::auto_ptr<TransmissionSource>(new TableOutputConfig());
    else {
        std::auto_ptr<TransmissionChooser> chooser(new TransmissionChooser());

        chooser->addChoice( new TableOutputConfig() );
        chooser->addChoice( new ViewerModule() );
        chooser->addChoice( new ProgressChoice() );
        chooser->addChoice( new CountChoice() );
        chooser->addChoice( new CrankshaftModule(*this) );
        chooser->addChoice( new EmissionTracker::Config(*this) );

        return std::auto_ptr<TransmissionSource>(chooser);
    }
}

}
