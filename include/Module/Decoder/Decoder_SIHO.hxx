#include <string>
#include <limits>
#include <sstream>
#include <algorithm>

#include "Tools/Exception/exception.hpp"
#include "Module/Module.hpp"
#include "Module/Decoder/Decoder_SIHO.hpp"

namespace aff3ct
{
namespace module
{

template <typename B, typename R>
Task& Decoder_SIHO<B,R>
::operator[](const dec::tsk t)
{
	return Module::operator[]((size_t)t);
}

template <typename B, typename R>
Socket& Decoder_SIHO<B,R>
::operator[](const dec::sck::decode_hiho s)
{
	return Module::operator[]((size_t)dec::tsk::decode_hiho)[(size_t)s];
}

template <typename B, typename R>
Socket& Decoder_SIHO<B,R>
::operator[](const dec::sck::decode_hiho_cw s)
{
	return Module::operator[]((size_t)dec::tsk::decode_hiho_cw)[(size_t)s];
}

template <typename B, typename R>
Socket& Decoder_SIHO<B,R>
::operator[](const dec::sck::decode_siho s)
{
	return Module::operator[]((size_t)dec::tsk::decode_siho)[(size_t)s];
}

template <typename B, typename R>
Socket& Decoder_SIHO<B,R>
::operator[](const dec::sck::decode_siho_cw s)
{
	return Module::operator[]((size_t)dec::tsk::decode_siho_cw)[(size_t)s];
}

template <typename B, typename R>
Decoder_SIHO<B,R>
::Decoder_SIHO(const int K, const int N)
: Decoder_HIHO<B>(K, N),
  Y_N(N * this->get_n_frames_per_wave())
{
	const std::string name = "Decoder_SIHO";
	this->set_name(name);

	auto &p1 = this->create_task("decode_siho", (int)dec::tsk::decode_siho);
	auto p1s_Y_N = this->template create_socket_in <R>(p1, "Y_N", this->N);
	auto p1s_V_K = this->template create_socket_out<B>(p1, "V_K", this->K);
	this->create_codelet(p1, [p1s_Y_N, p1s_V_K](Module &m, Task &t, const size_t frame_id) -> int
	{
		auto &dec = static_cast<Decoder_SIHO<B,R>&>(m);

		auto ret = dec._decode_siho(static_cast<R*>(t[p1s_Y_N].get_dataptr()),
		                            static_cast<B*>(t[p1s_V_K].get_dataptr()),
		                            frame_id);

		if (dec.is_auto_reset())
			dec._reset(frame_id);

		return ret;
	});
	this->register_timer(p1, "load");
	this->register_timer(p1, "decode");
	this->register_timer(p1, "store");
	this->register_timer(p1, "total");

	auto &p2 = this->create_task("decode_siho_cw", (int)dec::tsk::decode_siho_cw);
	auto p2s_Y_N = this->template create_socket_in <R>(p2, "Y_N", this->N);
	auto p2s_V_N = this->template create_socket_out<B>(p2, "V_N", this->N);
	this->create_codelet(p2, [p2s_Y_N, p2s_V_N](Module &m, Task &t, const size_t frame_id) -> int
	{
		auto &dec = static_cast<Decoder_SIHO<B,R>&>(m);

		auto ret = dec._decode_siho_cw(static_cast<R*>(t[p2s_Y_N].get_dataptr()),
		                               static_cast<B*>(t[p2s_V_N].get_dataptr()),
		                               frame_id);

		if (dec.is_auto_reset())
			dec._reset(frame_id);

		return ret;
	});
	this->register_timer(p2, "load");
	this->register_timer(p2, "decode");
	this->register_timer(p2, "store");
	this->register_timer(p2, "total");
}

template <typename B, typename R>
Decoder_SIHO<B,R>* Decoder_SIHO<B,R>
::clone() const
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

// template <typename B, typename R>
// template <class AR, class AB>
// int Decoder_SIHO<B,R>
// ::decode_siho(const std::vector<R,AR>& Y_N, std::vector<B,AB>& V_K, const int frame_id)
// {
// 	if (this->N * this->n_frames != (int)Y_N.size())
// 	{
// 		std::stringstream message;
// 		message << "'Y_N.size()' has to be equal to 'N' * 'n_frames' ('Y_N.size()' = " << Y_N.size()
// 		        << ", 'N' = " << this->N << ", 'n_frames' = " << this->n_frames << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	if (this->K * this->n_frames != (int)V_K.size())
// 	{
// 		std::stringstream message;
// 		message << "'V_K.size()' has to be equal to 'K' * 'n_frames' ('V_K.size()' = " << V_K.size()
// 		        << ", 'K' = " << this->K << ", 'n_frames' = " << this->n_frames << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	if (frame_id != -1 && frame_id >= this->n_frames)
// 	{
// 		std::stringstream message;
// 		message << "'frame_id' has to be equal to '-1' or to be smaller than 'n_frames' ('frame_id' = "
// 		        << frame_id << ", 'n_frames' = " << this->n_frames << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	return this->decode_siho(Y_N.data(), V_K.data(), frame_id);
// }

template <typename B, typename R>
template <class AR, class AB>
int Decoder_SIHO<B,R>
::decode_siho(const std::vector<R,AR>& Y_N, std::vector<B,AB>& V_K, const int frame_id, const bool managed_memory)
{
	(*this)[dec::sck::decode_siho::Y_N].bind(Y_N);
	(*this)[dec::sck::decode_siho::V_K].bind(V_K);
	return (*this)[dec::tsk::decode_siho].exec(frame_id, managed_memory);
}

// template <typename B, typename R>
// int Decoder_SIHO<B,R>
// ::decode_siho(const R *Y_N, B *V_K, const int frame_id)
// {
// 	int status = 0;
// 	if (frame_id < 0 || this->simd_inter_frame_level == 1)
// 	{
// 		const auto w_start = (frame_id < 0) ? 0 : frame_id % this->n_dec_waves;
// 		const auto w_stop  = (frame_id < 0) ? this->n_dec_waves : w_start +1;

// 		auto w = 0;
// 		for (w = w_start; w < w_stop -1; w++)
// 		{
// 			status <<= this->simd_inter_frame_level;
// 			auto s = this->_decode_siho(Y_N + w * this->N * this->simd_inter_frame_level,
// 			                            V_K + w * this->K * this->simd_inter_frame_level,
// 			                            w * this->simd_inter_frame_level);
// 			status |= s;

// 			if (this->is_auto_reset())
// 				this->_reset(w * this->simd_inter_frame_level);
// 		}

// 		if (this->n_inter_frame_rest == 0)
// 		{
// 			status <<= this->simd_inter_frame_level;
// 			auto s = this->_decode_siho(Y_N + w * this->N * this->simd_inter_frame_level,
// 			                            V_K + w * this->K * this->simd_inter_frame_level,
// 			                            w * this->simd_inter_frame_level);
// 			status |= s;

// 			if (this->is_auto_reset())
// 				this->_reset(w * this->simd_inter_frame_level);
// 		}
// 		else
// 		{
// 			const auto waves_off1 = w * this->simd_inter_frame_level * this->N;
// 			std::copy(Y_N + waves_off1,
// 			          Y_N + waves_off1 + this->n_inter_frame_rest * this->N,
// 			          this->Y_N.begin());

// 			status <<= this->simd_inter_frame_level;
// 			auto s = this->_decode_siho(this->Y_N.data(), this->V_KN.data(), w * this->simd_inter_frame_level);
// 			status |= s;

// 			if (this->is_auto_reset())
// 				this->_reset(w * this->simd_inter_frame_level);

// 			const auto waves_off2 = w * this->simd_inter_frame_level * this->K;
// 			std::copy(this->V_KN.begin(),
// 			          this->V_KN.begin() + this->n_inter_frame_rest * this->K,
// 			          V_K + waves_off2);
// 		}
// 	}
// 	else
// 	{
// 		const auto w = (frame_id % this->n_frames) / this->simd_inter_frame_level;
// 		const auto w_pos = frame_id % this->simd_inter_frame_level;

// 		std::fill(this->Y_N.begin(), this->Y_N.end(), (R)0);
// 		std::copy(Y_N + ((frame_id % this->n_frames) + 0) * this->N,
// 		          Y_N + ((frame_id % this->n_frames) + 1) * this->N,
// 		          this->Y_N.begin() + w_pos * this->N);

// 		status <<= this->simd_inter_frame_level;
// 		auto s = this->_decode_siho(this->Y_N.data(), this->V_KN.data(), w * this->simd_inter_frame_level);
// 		status |= s;

// 		if (this->is_auto_reset())
// 			this->_reset(w * this->simd_inter_frame_level);

// 		std::copy(this->V_KN.begin() + (w_pos +0) * this->K,
// 		          this->V_KN.begin() + (w_pos +1) * this->K,
// 		          V_K + (frame_id % this->n_frames) * this->K);
// 	}
// 	return status & this->mask;
// }

template <typename B, typename R>
int Decoder_SIHO<B,R>
::decode_siho(const R *Y_N, B *V_K, const int frame_id, const bool managed_memory)
{
	(*this)[dec::sck::decode_siho::Y_N].bind(Y_N);
	(*this)[dec::sck::decode_siho::V_K].bind(V_K);
	return (*this)[dec::tsk::decode_siho].exec(frame_id, managed_memory);
}

// template <typename B, typename R>
// template <class AR, class AB>
// int Decoder_SIHO<B,R>
// ::decode_siho_cw(const std::vector<R,AR>& Y_N, std::vector<B,AB>& V_N, const int frame_id)
// {
// 	if (this->N * this->n_frames != (int)Y_N.size())
// 	{
// 		std::stringstream message;
// 		message << "'Y_N.size()' has to be equal to 'N' * 'n_frames' ('Y_N.size()' = " << Y_N.size()
// 		        << ", 'N' = " << this->N << ", 'n_frames' = " << this->n_frames << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	if (this->N * this->n_frames != (int)V_N.size())
// 	{
// 		std::stringstream message;
// 		message << "'V_N.size()' has to be equal to 'N' * 'n_frames' ('V_N.size()' = " << V_N.size()
// 		        << ", 'N' = " << this->N << ", 'n_frames' = " << this->n_frames << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	if (frame_id != -1 && frame_id >= this->n_frames)
// 	{
// 		std::stringstream message;
// 		message << "'frame_id' has to be equal to '-1' or to be smaller than 'n_frames' ('frame_id' = "
// 		        << frame_id << ", 'n_frames' = " << this->n_frames << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	return this->decode_siho_cw(Y_N.data(), V_N.data(), frame_id);
// }

template <typename B, typename R>
template <class AR, class AB>
int Decoder_SIHO<B,R>
::decode_siho_cw(const std::vector<R,AR>& Y_N, std::vector<B,AB>& V_N, const int frame_id, const bool managed_memory)
{
	(*this)[dec::sck::decode_siho_cw::Y_N].bind(Y_N);
	(*this)[dec::sck::decode_siho_cw::V_N].bind(V_N);
	return (*this)[dec::tsk::decode_siho_cw].exec(frame_id, managed_memory);
}

// template <typename B, typename R>
// int Decoder_SIHO<B,R>
// ::decode_siho_cw(const R *Y_N, B *V_N, const int frame_id)
// {
// 	int status = 0;
// 	if (frame_id < 0 || this->simd_inter_frame_level == 1)
// 	{
// 		const auto w_start = (frame_id < 0) ? 0 : frame_id % this->n_dec_waves;
// 		const auto w_stop  = (frame_id < 0) ? this->n_dec_waves : w_start +1;

// 		auto w = 0;
// 		for (w = w_start; w < w_stop -1; w++)
// 		{
// 			status <<= this->simd_inter_frame_level;
// 			auto s = this->_decode_siho_cw(Y_N + w * this->N * this->simd_inter_frame_level,
// 			                               V_N + w * this->N * this->simd_inter_frame_level,
// 			                               w * this->simd_inter_frame_level);
// 			status |= s;

// 			if (this->is_auto_reset())
// 				this->_reset(w * this->simd_inter_frame_level);
// 		}

// 		if (this->n_inter_frame_rest == 0)
// 		{
// 			status <<= this->simd_inter_frame_level;
// 			auto s = this->_decode_siho_cw(Y_N + w * this->N * this->simd_inter_frame_level,
// 			                               V_N + w * this->N * this->simd_inter_frame_level,
// 			                               w * this->simd_inter_frame_level);
// 			status |= s;

// 			if (this->is_auto_reset())
// 				this->_reset(w * this->simd_inter_frame_level);
// 		}
// 		else
// 		{
// 			const auto waves_off1 = w * this->simd_inter_frame_level * this->N;
// 			std::copy(Y_N + waves_off1,
// 			          Y_N + waves_off1 + this->n_inter_frame_rest * this->N,
// 			          this->Y_N.begin());

// 			status <<= this->simd_inter_frame_level;
// 			auto s = this->_decode_siho_cw(this->Y_N.data(), this->V_KN.data(), w * this->simd_inter_frame_level);
// 			status |= s;

// 			if (this->is_auto_reset())
// 				this->_reset(w * this->simd_inter_frame_level);

// 			const auto waves_off2 = w * this->simd_inter_frame_level * this->N;
// 			std::copy(this->V_KN.begin(),
// 			          this->V_KN.begin() + this->n_inter_frame_rest * this->N,
// 			          V_N + waves_off2);
// 		}
// 	}
// 	else
// 	{
// 		const auto w = (frame_id % this->n_frames) / this->simd_inter_frame_level;
// 		const auto w_pos = frame_id % this->simd_inter_frame_level;

// 		std::fill(this->Y_N.begin(), this->Y_N.end(), (R)0);
// 		std::copy(Y_N + ((frame_id % this->n_frames) + 0) * this->N,
// 		          Y_N + ((frame_id % this->n_frames) + 1) * this->N,
// 		          this->Y_N.begin() + w_pos * this->N);

// 		status <<= this->simd_inter_frame_level;
// 		auto s = this->_decode_siho_cw(this->Y_N.data(), this->V_KN.data(), w * this->simd_inter_frame_level);
// 		status |= s;

// 		if (this->is_auto_reset())
// 			this->_reset(w * this->simd_inter_frame_level);

// 		std::copy(this->V_KN.begin() + (w_pos +0) * this->N,
// 		          this->V_KN.begin() + (w_pos +1) * this->N,
// 		          V_N + (frame_id % this->n_frames) * this->N);
// 	}
// 	return status & this->mask;
// }

template <typename B, typename R>
int Decoder_SIHO<B,R>
::decode_siho_cw(const R *Y_N, B *V_N, const int frame_id, const bool managed_memory)
{
	(*this)[dec::sck::decode_siho_cw::Y_N].bind(Y_N);
	(*this)[dec::sck::decode_siho_cw::V_N].bind(V_N);
	return (*this)[dec::tsk::decode_siho_cw].exec(frame_id, managed_memory);
}

template <typename B, typename R>
int Decoder_SIHO<B,R>
::_decode_siho(const R *Y_N, B *V_K, const size_t frame_id)
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template <typename B, typename R>
int Decoder_SIHO<B,R>
::_decode_siho_cw(const R *Y_N, B *V_N, const size_t frame_id)
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template <typename B, typename R>
int Decoder_SIHO<B,R>
::_decode_hiho(const B *Y_N, B *V_K, const size_t frame_id)
{
	for (size_t i = 0; i < (size_t)this->N * this->get_n_frames_per_wave(); i++)
		this->Y_N[i] = Y_N[i] ? (R)-1 : (R)1;
	return this->_decode_siho(this->Y_N.data(), V_K, frame_id);
}

template <typename B, typename R>
int Decoder_SIHO<B,R>
::_decode_hiho_cw(const B *Y_N, B *V_N, const size_t frame_id)
{
	for (size_t i = 0; i < (size_t)this->N * this->get_n_frames_per_wave(); i++)
		this->Y_N[i] = Y_N[i] ? (R)-1 : (R)1;
	return this->_decode_siho_cw(this->Y_N.data(), V_N, frame_id);
}

template <typename B, typename R>
void Decoder_SIHO<B,R>
::set_n_frames_per_wave(const size_t n_frames_per_wave)
{
	const auto old_n_frames_per_wave = this->get_n_frames_per_wave();
	if (old_n_frames_per_wave != n_frames_per_wave)
	{
		Decoder_HIHO<B>::set_n_frames_per_wave(n_frames_per_wave);
		this->Y_N.resize(this->N * n_frames_per_wave);
	}
}

}
}

