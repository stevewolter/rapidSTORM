#ifndef DSTORM_JOB_H
#define DSTORM_JOB_H

namespace simparm { class Node; }

namespace dStorm {

struct Job {
    virtual ~Job() {}
    virtual simparm::Node& get_config() = 0;
    virtual void stop() = 0;
};

}

#endif
