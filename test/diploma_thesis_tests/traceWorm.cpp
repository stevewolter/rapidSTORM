#include <dStorm/CarConfig.h>
#include <dStorm/Car.h>
#include <dStorm/output/Localization.h>
#include <list>
#include <algorithm>
#include <foreach.h>
#include <math.h>

using namespace dStorm;
using namespace std;

bool cmpByImageNumber(const Localization &a, const Localization &b) {
   return (a.getImageNumber() < b.getImageNumber());
}

class WormTracer : public Passmission {
  private:
    vector<double> x, y, w;
    double sigma;
    unsigned int max;
  public:
    WormTracer(double s) throw() : x(0), y(0), w(0), sigma(s) {}
    ~WormTracer() throw() {}
    void announceStormSize(const Announcement &a) throw() { 
        int l = a.length;
        x.resize(l, 0); y.resize(l, 0); w.resize(l, 0);
        max = 0; 
    }
    Result receiveLocalizations(const Localization *first, int number, const Image &)
        throw()
    {
        if (number == 0) return KeepRunning;
        unsigned int num = first[0].getImageNumber();
        if (num > max) max = num;
        int tl = std::max(0, int(ceil( num - 5*sigma ))), 
            th = min<int>(x.size()-1, int(floor( num + 5*sigma)));

        for (int i = tl; i <= th; i++) {
            int dist = abs(i-int(num));
            double weight = exp( - (dist * dist) / (2*sigma*sigma) );
            for (int f = 0; f < number; f++) {
                x[i] += first[f].x() * weight;
                y[i] += first[f].y() * weight;
            }
            w[i] += number * weight;
        }
        return KeepRunning;
    }

    void carStarted(dStorm::CarConfig &, dStorm::Crankshaft&cs) throw() {
        cs.add(*this);
    }
    void deleteAllResults() throw() {
        for (unsigned int i = 0; i <= max; i++)
            x[i] = y[i] = w[i] = 0;
    }

    void carStopped() throw() {
        double xs[max+1], ys[max+1];
        double x0 = x[0] / w[0], y0 = y[0] / w[0];
        double minX = 0, minY = 0, maxX = 0, maxY = 0;
        for (unsigned int i = 0; i <= max; i++) {
            xs[i] = x[i]/w[i]-x0;
            ys[i] = y[i]/w[i]-y0;
            minX = min(minX, xs[i]), minY = min(minY, ys[i]);
            maxX = std::max(maxX, xs[i]), maxY = std::max(maxY, ys[i]);
        }

        cout << ceil(maxX) << " " << ceil(maxY) << " " << max << "\n";
        for (unsigned int i = 0; i <= max; i++)
            cout << x[i] / w[i] - x0 - minX << " " 
                 << y[i] / w[i] - y0 -minY
                 << " " << i << "\n";
    }

    auto_ptr<Passenger> clone() const throw() 
        { return auto_ptr<Passenger>(new WormTracer(sigma)); }

    const char *getName() throw() { return "WormTracer"; }
};

int main(int argc, char *argv[]) {
   CarConfig config;
   EntryDouble sigma;
   sigma.setName("Sigma");
   sigma.setDesc("Standard deviation for the exponantially weighted "
                 "average position of the worm.");
   sigma = 6;
   config.registerEntries(config);
   config.register_entry(&sigma);
   config.readConfig(argc, argv);

   Car car(config);
   car.addPassenger(new WormTracer(sigma()));
   car.run();

   return 0;
}
