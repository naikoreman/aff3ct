#include <cassert>

#include "Module/Module.hpp"

namespace aff3ct
{
namespace module
{
Task& Module
::operator[](const size_t id)
{
	assert(id < this->tasks_with_nullptr.size());
	assert(this->tasks_with_nullptr[id] != nullptr);

	return *this->tasks_with_nullptr[id];
}

template <typename T>
inline size_t Module
::create_socket_in(Task& task, const std::string &name, const size_t n_elmts)
{
	return task.template create_socket_in<T>(name, n_elmts * this->n_frames);
}

template <typename T>
inline size_t Module
::create_socket_in_out(Task& task, const std::string &name, const size_t n_elmts)
{
	return task.template create_socket_in_out<T>(name, n_elmts * this->n_frames);
}

template <typename T>
inline size_t Module
::create_socket_out(Task& task, const std::string &name, const size_t n_elmts)
{
	return task.template create_socket_out<T>(name, n_elmts * this->n_frames);
}

size_t Module
::get_n_frames() const
{
	return this->n_frames;
}

size_t Module
::get_n_frames_per_wave() const
{
	return this->n_frames_per_wave;
}

size_t Module
::get_n_waves() const
{
	return this->n_waves;
}

size_t Module
::get_n_frames_per_wave_rest() const
{
	return this->n_frames_per_wave_rest;
}

bool Module
::is_single_wave() const
{
	return this->single_wave;
}

}
}
