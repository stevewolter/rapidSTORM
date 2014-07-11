#ifndef DSTORM_JOB_QUEUE_H
#define DSTORM_JOB_QUEUE_H

#include <array>

#include <boost/exception_ptr.hpp>
#include <boost/optional/optional.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

#include "output/LocalizedImage.h"

namespace dStorm {
namespace job {

class Queue {
    boost::mutex ring_buffer_mutex;
    boost::condition producer_can_continue, consumer_can_continue;
    int next_output;
    std::array< boost::optional<output::LocalizedImage>, 64 > ring_buffer;
    int producer_count;
    boost::exception_ptr error;
    bool interruption;

    int ring() const { return next_output % ring_buffer.size(); }
  public:
    typedef output::LocalizedImage value_type;
    typedef const output::LocalizedImage& const_reference;

    Queue( frame_index first_image, int producer_count );
    ~Queue();

    void push( const_reference );
    void push_back( const_reference i ) { push(i); }

    bool has_more_input();
    const output::LocalizedImage& front() const;
    void pop();

    void notice_error( boost::exception_ptr );
    void rethrow_exception();

    void producer_finished();
    void interrupt_producers();
};

}
}

#endif
