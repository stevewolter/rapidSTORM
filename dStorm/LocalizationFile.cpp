#define DSTORM_LOCALIZATIONFILE_CPP
#include "LocalizationFile.h"
#include <string.h>

using namespace std;

namespace dStorm {

static void printRemainder(ostream &o, float arg) {
   char buffer[12];
   sprintf(buffer, "%1.6f", double(arg));
   const char *start = strrchr(buffer, '.') + 1;
   o << start;
}

static void printFit(const Localization &f, ostream &file) {
      file << f.getXLow() << ".";
      printRemainder(file, f.getXR());
      file << " ";
      file << f.getYLow() << ".";
      printRemainder(file, f.getYR());
      file << " " << f.getImageNumber();
      file << " " << f.getStrength();
      file << "\n";
}

void LocalizationFile::open() throw() {
    if ( filename != "-" ) {
        fileKeeper.reset( new ofstream( filename.c_str(), 
                                    ios_base::out | ios_base::trunc ) );
        file = fileKeeper.get();
    } else
        file = &cout;
    *file << w << " " << h << " " << l << " 0" << "\n";
}

Transmission::AdditionalData
LocalizationFile::announceStormSize(int w, int h, int l) throw() {
    ost::MutexLock lock(mutex);
    this->w = w;
    this->h = h;
    this->l = l;

    open();

    return NoData;
}

Transmission::Result LocalizationFile::receiveLocalizations(const EngineResult &er) 
        throw()
{
    ost::MutexLock lock(mutex);
    for (int i = 0; i < er.number; i++)
        printFit(er.first[i], *file);
    return (*file) ? KeepRunning : RemoveTransmission;
}


void LocalizationFile::deleteAllResults() throw() {
    ost::MutexLock lock(mutex);
    fileKeeper.reset(NULL);
    open();
}

void LocalizationFile::finish() throw() {
    ost::MutexLock lock(mutex);
    if (fileKeeper.get() != NULL) fileKeeper->close();
    fileKeeper.reset(NULL);
}

LocalizationFile::LocalizationFile(const std::string &filename) throw()
: ConfigObject("LocalizationFile", "File output status"),
  filename(filename) {}

LocalizationFile::~LocalizationFile() throw() {}

}
