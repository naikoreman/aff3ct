#ifdef CHANNEL_MKL

#ifndef EVENT_GENERATOR_MKL_HPP
#define EVENT_GENERATOR_MKL_HPP

#include <mkl_vsl.h>

#include "../Event_generator.hpp"

namespace aff3ct
{
namespace tools
{

template <typename R = float>
class Event_generator_MKL : public Event_generator<R>
{
private:
	VSLStreamStatePtr stream_state;
	bool              is_stream_alloc;

public:
	explicit Event_generator_MKL(const int seed = 0);

	virtual ~Event_generator_MKL();

	virtual void set_seed(const int seed);

	virtual void generate(event_type *draw, const unsigned length, const R event_probability);
};

}
}

#endif //EVENT_GENERATOR_MKL_HPP

#endif // MKL