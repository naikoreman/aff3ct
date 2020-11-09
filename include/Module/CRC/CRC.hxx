#include <string>
#include <sstream>

#include "Tools/Exception/exception.hpp"
#include "Module/CRC/CRC.hpp"

namespace aff3ct
{
namespace module
{

template <typename B>
Task& CRC<B>
::operator[](const crc::tsk t)
{
	return Module::operator[]((size_t)t);
}

template <typename B>
Socket& CRC<B>
::operator[](const crc::sck::build s)
{
	return Module::operator[]((size_t)crc::tsk::build)[(size_t)s];
}

template <typename B>
Socket& CRC<B>
::operator[](const crc::sck::extract s)
{
	return Module::operator[]((size_t)crc::tsk::extract)[(size_t)s];
}

template <typename B>
Socket& CRC<B>
::operator[](const crc::sck::check s)
{
	return Module::operator[]((size_t)crc::tsk::check)[(size_t)s];
}

template <typename B>
CRC<B>
::CRC(const int K, const int size)
: Module(), K(K), size(size)
{
	const std::string name = "CRC";
	this->set_name(name);
	this->set_short_name(name);

	if (K <= 0)
	{
		std::stringstream message;
		message << "'K' has to be greater than 0 ('K' = " << K << ").";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	auto &p1 = this->create_task("build");
	auto p1s_U_K1 = this->template create_socket_in <B>(p1, "U_K1", this->K             );
	auto p1s_U_K2 = this->template create_socket_out<B>(p1, "U_K2", this->K + this->size);
	this->create_codelet(p1, [p1s_U_K1, p1s_U_K2](Module &m, Task &t, const size_t frame_id) -> int
	{
		auto &crc = static_cast<CRC<B>&>(m);

		crc._build(static_cast<B*>(t[p1s_U_K1].get_dataptr()),
		           static_cast<B*>(t[p1s_U_K2].get_dataptr()),
		           frame_id);

		return status_t::SUCCESS;
	});

	auto &p2 = this->create_task("extract");
	auto p2s_V_K1 = this->template create_socket_in <B>(p2, "V_K1", this->K + this->size);
	auto p2s_V_K2 = this->template create_socket_out<B>(p2, "V_K2", this->K             );
	this->create_codelet(p2, [p2s_V_K1, p2s_V_K2](Module &m, Task &t, const size_t frame_id) -> int
	{
		auto &crc = static_cast<CRC<B>&>(m);

		crc._extract(static_cast<B*>(t[p2s_V_K1].get_dataptr()),
		             static_cast<B*>(t[p2s_V_K2].get_dataptr()),
		             frame_id);

		return status_t::SUCCESS;
	});

	auto &p3 = this->create_task("check");
	auto p3s_V_K = this->template create_socket_in<B>(p3, "V_K", this->K + this->size);
	this->create_codelet(p3, [p3s_V_K](Module &m, Task &t, const size_t frame_id) -> int
	{
		auto &crc = static_cast<CRC<B>&>(m);

		auto ret = crc._check(static_cast<B*>(t[p3s_V_K].get_dataptr()), frame_id);

		return ret ? status_t::SUCCESS : status_t::FAILURE_STOP;
	});

	auto &p4 = this->create_task("check_packed");
	auto p4s_V_K = this->template create_socket_in<B>(p4, "V_K", this->K + this->size);
	this->create_codelet(p4, [p4s_V_K](Module &m, Task &t, const size_t frame_id) -> int
	{
		auto &crc = static_cast<CRC<B>&>(m);

		auto ret = crc._check_packed(static_cast<B*>(t[p4s_V_K].get_dataptr()), frame_id);

		return ret ? status_t::SUCCESS : status_t::FAILURE_STOP;
	});
}

template <typename B>
CRC<B>* CRC<B>
::clone() const
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template <typename B>
int CRC<B>
::get_K() const
{
	return this->K;
}


template <typename B>
int CRC<B>
::get_size()
{
	return size;
}


template <typename B>
template <class A>
void CRC<B>
::build(const std::vector<B,A>& U_K1, std::vector<B,A>& U_K2, const int frame_id, const bool managed_memory)
{
	(*this)[crc::sck::build::U_K1].bind(U_K1);
	(*this)[crc::sck::build::U_K2].bind(U_K2);
	(*this)[crc::tsk::build].exec(frame_id, managed_memory);
}

template <typename B>
void CRC<B>
::build(const B *U_K1, B *U_K2, const int frame_id, const bool managed_memory)
{
	(*this)[crc::sck::build::U_K1].bind(U_K1);
	(*this)[crc::sck::build::U_K2].bind(U_K2);
	(*this)[crc::tsk::build].exec(frame_id, managed_memory);
}

template <typename B>
template <class A>
void CRC<B>
::extract(const std::vector<B,A>& V_K1, std::vector<B,A>& V_K2, const int frame_id, const bool managed_memory)
{
	(*this)[crc::sck::extract::V_K1].bind(V_K1);
	(*this)[crc::sck::extract::V_K2].bind(V_K2);
	(*this)[crc::tsk::extract].exec(frame_id, managed_memory);
}

template <typename B>
void CRC<B>
::extract(const B *V_K1, B *V_K2, const int frame_id, const bool managed_memory)
{
	(*this)[crc::sck::extract::V_K1].bind(V_K1);
	(*this)[crc::sck::extract::V_K2].bind(V_K2);
	(*this)[crc::tsk::extract].exec(frame_id, managed_memory);
}

// template <typename B>
// template <class A>
// bool CRC<B>
// ::check(const std::vector<B,A>& V_K, const int n_frames, const int frame_id)
// {
// 	if (n_frames <= 0 && n_frames != -1)
// 	{
// 		std::stringstream message;
// 		message << "'n_frames' has to be greater than 0 or equal to -1 ('n_frames' = " << n_frames << ").";
// 		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	if (frame_id != -1 && frame_id >= (n_frames ? n_frames : this->n_frames))
// 	{
// 		std::stringstream message;
// 		message << "'frame_id' has to be equal to '-1' or to be smaller than 'n_frames' ('frame_id' = "
// 		        << frame_id << ", 'n_frames' = " << (n_frames ? n_frames : this->n_frames) << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	if ((this->K + (int)this->get_size()) *       n_frames != (int)V_K.size() &&
// 	    (this->K + (int)this->get_size()) * this->n_frames != (int)V_K.size())
// 	{
// 		std::stringstream message;
// 		message << "'V_K.size()' has to be equal to ('K' + 'size') * 'n_frames' ('V_K.size()' = " << V_K.size()
// 		        << ", 'K' = " << this->K
// 		        << ", 'n_frames' = " << (n_frames != -1 ? n_frames : this->n_frames) << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	return this->check(V_K.data(), n_frames, frame_id);
// }

template <typename B>
template <class A>
bool CRC<B>
::check(const std::vector<B,A>& V_K, const int frame_id, const bool managed_memory)
{
	(*this)[crc::sck::check::V_K].bind(V_K);
	auto ret = (*this)[crc::tsk::check].exec(frame_id, managed_memory);
	return !ret;
}

// template <typename B>
// bool CRC<B>
// ::check(const B *V_K, const int n_frames, const int frame_id)
// {
// 	const int real_n_frames = (n_frames != -1) ? n_frames : this->n_frames;

// 	const auto f_start = (frame_id < 0) ? 0 : frame_id % real_n_frames;
// 	const auto f_stop  = (frame_id < 0) ? real_n_frames : f_start +1;

// 	auto f = f_start;
// 	while (f < f_stop && this->_check(V_K + f * (this->K + this->get_size()), f))
// 		f++;

// 	return f == f_stop;
// }

template <typename B>
bool CRC<B>
::check(const B *V_K, const int frame_id, const bool managed_memory)
{
	(*this)[crc::sck::check::V_K].bind(V_K);
	auto ret = (*this)[crc::tsk::check].exec(frame_id, managed_memory);
	return !ret;
}

// template <typename B>
// template <class A>
// bool CRC<B>
// ::check_packed(const std::vector<B,A>& V_K, const int n_frames, const int frame_id)
// {
// 	if (n_frames <= 0 && n_frames != -1)
// 	{
// 		std::stringstream message;
// 		message << "'n_frames' has to be greater than 0 or equal to -1 ('n_frames' = " << n_frames << ").";
// 		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	if (frame_id != -1 && frame_id >= (n_frames ? n_frames : this->n_frames))
// 	{
// 		std::stringstream message;
// 		message << "'frame_id' has to be equal to '-1' or to be smaller than 'n_frames' ('frame_id' = "
// 		        << frame_id << ", 'n_frames' = " << (n_frames ? n_frames : this->n_frames) << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	if ((this->K + (int)this->get_size()) *       n_frames != (int)V_K.size() &&
// 	    (this->K + (int)this->get_size()) * this->n_frames != (int)V_K.size())
// 	{
// 		std::stringstream message;
// 		message << "'V_K.size()' has to be equal to ('K' + 'size') * 'n_frames' ('V_K.size()' = " << V_K.size()
// 		        << ", 'K' = " << this->K
// 		        << ", 'n_frames' = " << (n_frames != -1 ? n_frames : this->n_frames) << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	return this->check_packed(V_K.data(), n_frames, frame_id);
// }

template <typename B>
template <class A>
bool CRC<B>
::check_packed(const std::vector<B,A>& V_K, const int frame_id, const bool managed_memory)
{
	(*this)[crc::sck::check_packed::V_K].bind(V_K);
	auto ret = (*this)[crc::tsk::check_packed].exec(frame_id, managed_memory);
	return !ret;
}

// template <typename B>
// bool CRC<B>
// ::check_packed(const B *V_K, const int n_frames, const int frame_id)
// {
// 	const int real_n_frames = (n_frames != -1) ? n_frames : this->n_frames;

// 	const auto f_start = (frame_id < 0) ? 0 : frame_id % real_n_frames;
// 	const auto f_stop  = (frame_id < 0) ? real_n_frames : f_start +1;

// 	auto f = f_start;
// 	while (f < f_stop && this->_check_packed(V_K + f * (this->K + this->get_size()), f))
// 		f++;

// 	return f == f_stop;
// }

template <typename B>
bool CRC<B>
::check_packed(const B *V_K, const int frame_id, const bool managed_memory)
{
	(*this)[crc::sck::check_packed::V_K].bind(V_K);
	auto ret = (*this)[crc::tsk::check_packed].exec(frame_id, managed_memory);
	return !ret;
}

template <typename B>
void CRC<B>
::_build(const B *U_K1, B *U_K2, const size_t frame_id)
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template <typename B>
void CRC<B>
::_extract(const B *V_K1, B *V_K2, const size_t frame_id)
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template <typename B>
bool CRC<B>
::_check(const B *V_K, const size_t frame_id)
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
	return false;
}

template <typename B>
bool CRC<B>
::_check_packed(const B *V_K, const size_t frame_id)
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
	return false;
}

}
}
