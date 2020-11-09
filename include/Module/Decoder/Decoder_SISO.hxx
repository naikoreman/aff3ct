#include <string>
#include <limits>
#include <sstream>
#include <algorithm>

#include "Tools/Exception/exception.hpp"
#include "Module/Module.hpp"
#include "Module/Decoder/Decoder_SISO.hpp"

namespace aff3ct
{
namespace module
{

template <typename B, typename R>
Task& Decoder_SISO<B,R>
::operator[](const dec::tsk t)
{
	return Module::operator[]((size_t)t);
}

template <typename B, typename R>
Socket& Decoder_SISO<B,R>
::operator[](const dec::sck::decode_hiho s)
{
	return Module::operator[]((size_t)dec::tsk::decode_hiho)[(size_t)s];
}

template <typename B, typename R>
Socket& Decoder_SISO<B,R>
::operator[](const dec::sck::decode_hiho_cw s)
{
	return Module::operator[]((size_t)dec::tsk::decode_hiho_cw)[(size_t)s];
}

template <typename B, typename R>
Socket& Decoder_SISO<B,R>
::operator[](const dec::sck::decode_siho s)
{
	return Module::operator[]((size_t)dec::tsk::decode_siho)[(size_t)s];
}

template <typename B, typename R>
Socket& Decoder_SISO<B,R>
::operator[](const dec::sck::decode_siho_cw s)
{
	return Module::operator[]((size_t)dec::tsk::decode_siho_cw)[(size_t)s];
}

template <typename B, typename R>
Socket& Decoder_SISO<B,R>
::operator[](const dec::sck::decode_siso s)
{
	return Module::operator[]((size_t)dec::tsk::decode_siso)[(size_t)s];
}

template <typename B, typename R>
Socket& Decoder_SISO<B,R>
::operator[](const dec::sck::decode_siso_alt s)
{
	return Module::operator[]((size_t)dec::tsk::decode_siso_alt)[(size_t)s];
}

template <typename B, typename R>
Decoder_SISO<B,R>
::Decoder_SISO(const int K, const int N)
: Decoder_SIHO<B,R>(K, N)
{
	const std::string name = "Decoder_SISO";
	this->set_name(name);

	auto &p1 = this->create_task("decode_siso", (int)dec::tsk::decode_siso);
	auto p1s_Y_N1 = this->template create_socket_in <R>(p1, "Y_N1", this->N);
	auto p1s_Y_N2 = this->template create_socket_out<R>(p1, "Y_N2", this->N);
	this->create_codelet(p1, [p1s_Y_N1, p1s_Y_N2](Module &m, Task &t, const size_t frame_id) -> int
	{
		auto &dec = static_cast<Decoder_SISO<B,R>&>(m);

		auto ret = dec._decode_siso(static_cast<R*>(t[p1s_Y_N1].get_dataptr()),
		                            static_cast<R*>(t[p1s_Y_N2].get_dataptr()),
		                            frame_id);

		if (dec.is_auto_reset())
			dec._reset(frame_id);

		return ret;
	});

	auto &p2 = this->create_task("decode_siso_alt", (int)dec::tsk::decode_siso_alt);
	auto p2s_sys = this->template create_socket_in <R>(p2, "sys", this->K + this->tail_length() / 2);
	auto p2s_par = this->template create_socket_in <R>(p2, "par", this->K + this->tail_length() / 2);
	auto p2s_ext = this->template create_socket_out<R>(p2, "ext", this->K);
	this->create_codelet(p2, [p2s_sys, p2s_par, p2s_ext](Module &m, Task &t, const size_t frame_id) -> int
	{
		auto &dec = static_cast<Decoder_SISO<B,R>&>(m);

		auto ret = dec._decode_siso_alt(static_cast<R*>(t[p2s_sys].get_dataptr()),
		                                static_cast<R*>(t[p2s_par].get_dataptr()),
		                                static_cast<R*>(t[p2s_ext].get_dataptr()),
		                                frame_id);

		if (dec.is_auto_reset())
			dec._reset(frame_id);

		return ret;
	});
}

template <typename B, typename R>
Decoder_SISO<B,R>* Decoder_SISO<B,R>
::clone() const
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

// template <typename B, typename R>
// template <class A>
// int Decoder_SISO<B,R>
// ::decode_siso(const std::vector<R,A> &Y_N1, std::vector<R,A> &Y_N2, const int frame_id)
// {
// 	if (this->N * this->n_frames != (int)Y_N1.size())
// 	{
// 		std::stringstream message;
// 		message << "'Y_N1.size()' has to be equal to 'N' * 'n_frames' ('Y_N1.size()' = " << Y_N1.size()
// 		        << ", 'N' = " << this->N << ", 'n_frames' = " << this->n_frames << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	if (this->N * this->n_frames != (int)Y_N2.size())
// 	{
// 		std::stringstream message;
// 		message << "'Y_N2.size()' has to be equal to 'N' * 'n_frames' ('Y_N2.size()' = " << Y_N2.size()
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

// 	return this->decode_siso(Y_N1.data(), Y_N2.data(), frame_id);
// }

template <typename B, typename R>
template <class A>
int Decoder_SISO<B,R>
::decode_siso(const std::vector<R,A> &Y_N1, std::vector<R,A> &Y_N2, const int frame_id, const bool managed_memory)
{
	(*this)[dec::sck::decode_siso::Y_N1].bind(Y_N1);
	(*this)[dec::sck::decode_siso::Y_N2].bind(Y_N2);
	return (*this)[dec::tsk::decode_siso].exec(frame_id, managed_memory);
}

// template <typename B, typename R>
// int Decoder_SISO<B,R>
// ::decode_siso(const R *Y_N1, R *Y_N2, const int frame_id)
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
// 			auto s = this->_decode_siso(Y_N1 + w * this->N * this->simd_inter_frame_level,
// 			                            Y_N2 + w * this->N * this->simd_inter_frame_level,
// 			                            w * this->simd_inter_frame_level);
// 			status |= s;

// 			if (this->is_auto_reset())
// 				this->_reset(w * this->simd_inter_frame_level);
// 		}

// 		if (this->n_inter_frame_rest == 0)
// 		{
// 			status <<= this->simd_inter_frame_level;
// 			auto s = this->_decode_siso(Y_N1 + w * this->N * this->simd_inter_frame_level,
// 			                            Y_N2 + w * this->N * this->simd_inter_frame_level,
// 			                            w * this->simd_inter_frame_level);
// 			status |= s;

// 			if (this->is_auto_reset())
// 				this->_reset(w * this->simd_inter_frame_level);
// 		}
// 		else
// 		{
// 			const auto waves_off1 = w * this->simd_inter_frame_level * this->N;
// 			std::copy(Y_N1 + waves_off1,
// 			          Y_N1 + waves_off1 + this->n_inter_frame_rest * this->N,
// 			          this->Y_N1.begin());

// 			status <<= this->simd_inter_frame_level;
// 			auto s = this->_decode_siso(this->Y_N1.data(), this->Y_N2.data(), w * this->simd_inter_frame_level);
// 			status |= s;

// 			if (this->is_auto_reset())
// 				this->_reset(w * this->simd_inter_frame_level);

// 			const auto waves_off2 = w * this->simd_inter_frame_level * this->N;
// 			std::copy(this->Y_N2.begin(),
// 			          this->Y_N2.begin() + this->n_inter_frame_rest * this->N,
// 			          Y_N2 + waves_off2);
// 		}
// 	}
// 	else
// 	{
// 		const auto w = (frame_id % this->n_frames) / this->simd_inter_frame_level;
// 		const auto w_pos = frame_id % this->simd_inter_frame_level;

// 		std::fill(this->Y_N1.begin(), this->Y_N1.end(), (R)0);
// 		std::copy(Y_N1 + ((frame_id % this->n_frames) + 0) * this->N,
// 		          Y_N1 + ((frame_id % this->n_frames) + 1) * this->N,
// 		          this->Y_N1.begin() + w_pos * this->N);

// 		status <<= this->simd_inter_frame_level;
// 		auto s = this->_decode_siso(this->Y_N1.data(), this->Y_N2.data(), w * this->simd_inter_frame_level);
// 		status |= s;

// 		if (this->is_auto_reset())
// 			this->_reset(w * this->simd_inter_frame_level);

// 		std::copy(this->Y_N2.begin() + (w_pos +0) * this->N,
// 		          this->Y_N2.begin() + (w_pos +1) * this->N,
// 		          Y_N2 + (frame_id % this->n_frames) * this->N);
// 	}
// 	return status & this->mask;
// }

template <typename B, typename R>
int Decoder_SISO<B,R>
::decode_siso(const R *Y_N1, R *Y_N2, const int frame_id, const bool managed_memory)
{
	(*this)[dec::sck::decode_siso::Y_N1].bind(Y_N1);
	(*this)[dec::sck::decode_siso::Y_N2].bind(Y_N2);
	return (*this)[dec::tsk::decode_siso].exec(frame_id, managed_memory);
}

// template <typename B, typename R>
// template <class A>
// int Decoder_SISO<B,R>
// ::decode_siso(const std::vector<R,A> &sys, const std::vector<R,A> &par, std::vector<R,A> &ext,
//               const int n_frames, const int frame_id)
// {
// 	if (n_frames != -1 && n_frames <= 0)
// 	{
// 		std::stringstream message;
// 		message << "'n_frames' has to be greater than 0 or equal to -1 ('n_frames' = " << n_frames << ").";
// 		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	const int real_n_frames = (n_frames != -1) ? n_frames : this->n_frames;

// 	if ((this->K + this->tail_length() / 2) * real_n_frames != (int)sys.size())
// 	{
// 		std::stringstream message;
// 		message << "'sys.size()' has to be equal to ('K' + 'tail_length()' / 2) * 'real_n_frames' "
// 		        << "('sys.size()' = " << sys.size()
// 		        << ", 'K' = " << this->K
// 		        << ", 'tail_length()' = " << this->tail_length()
// 		        << ", 'real_n_frames' = " << real_n_frames << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	if (((this->N - this->K) - this->tail_length() / 2) * real_n_frames != (int)par.size())
// 	{
// 		std::stringstream message;
// 		message << "'par.size()' has to be equal to (('N' - 'K') - 'tail_length()' / 2) * "
// 		        << "'real_n_frames' ('par.size()' = " << par.size()
// 		        << ", 'N' = " << this->N
// 		        << ", 'K' = " << this->K
// 		        << ", 'tail_length()' = " << this->tail_length()
// 		        << ", 'real_n_frames' = " << real_n_frames << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	if (this->K * real_n_frames != (int)ext.size())
// 	{
// 		std::stringstream message;
// 		message << "'ext.size()' has to be equal to 'K' * 'real_n_frames' ('ext.size()' = " << ext.size()
// 		        << ", 'K' = " << this->K << ", 'real_n_frames' = " << real_n_frames << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	if (frame_id != -1 && frame_id >= real_n_frames)
// 	{
// 		std::stringstream message;
// 		message << "'frame_id' has to be equal to '-1' or to be smaller than 'real_n_frames' ('frame_id' = "
// 		        << frame_id << ", 'real_n_frames' = " << real_n_frames << ").";
// 		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
// 	}

// 	return this->decode_siso(sys.data(), par.data(), ext.data(), real_n_frames, frame_id);
// }

template <typename B, typename R>
template <class A>
int Decoder_SISO<B,R>
::decode_siso_alt(const std::vector<R,A> &sys, const std::vector<R,A> &par, std::vector<R,A> &ext, const int frame_id,
                  const bool managed_memory)
{
	(*this)[dec::sck::decode_siso_alt::sys].bind(sys);
	(*this)[dec::sck::decode_siso_alt::par].bind(par);
	(*this)[dec::sck::decode_siso_alt::ext].bind(ext);
	return (*this)[dec::tsk::decode_siso_alt].exec(frame_id, managed_memory);
}

// template <typename B, typename R>
// int Decoder_SISO<B,R>
// ::decode_siso(const R *sys, const R *par, R *ext, const int n_frames, const int frame_id)
// {
// 	int status = 0;
// 	const int real_n_frames = (n_frames != -1) ? n_frames : this->n_frames;

// 	const auto n_dec_waves_siso = real_n_frames / this->simd_inter_frame_level;

// 	const auto w_start = (frame_id < 0) ? 0 : frame_id % n_dec_waves_siso;
// 	const auto w_stop  = (frame_id < 0) ? n_dec_waves_siso : w_start +1;

// 	for (auto w = w_start; w < w_stop; w++)
// 	{
// 		status <<= this->simd_inter_frame_level;
// 		auto s = this->_decode_siso(sys + w * (          this->K) * this->simd_inter_frame_level,
// 		                            par + w * (this->N - this->K) * this->simd_inter_frame_level,
// 		                            ext + w * (          this->K) * this->simd_inter_frame_level,
// 		                            w * this->simd_inter_frame_level);
// 		status |= s;

// 		if (this->is_auto_reset())
// 			this->_reset(w * this->simd_inter_frame_level);
// 	}
// 	return status & this->mask;
// }

template <typename B, typename R>
int Decoder_SISO<B,R>
::decode_siso_alt(const R *sys, const R *par, R *ext, const int frame_id, const bool managed_memory)
{
	(*this)[dec::sck::decode_siso_alt::sys].bind(sys);
	(*this)[dec::sck::decode_siso_alt::par].bind(par);
	(*this)[dec::sck::decode_siso_alt::ext].bind(ext);
	return (*this)[dec::tsk::decode_siso_alt].exec(frame_id, managed_memory);
}

template <typename B, typename R>
int Decoder_SISO<B,R>
::tail_length() const
{
	return 0;
}

template <typename B, typename R>
int Decoder_SISO<B,R>
::_decode_siso(const R *Y_N1, R *Y_N2, const size_t frame_id)
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template <typename B, typename R>
int Decoder_SISO<B,R>
::_decode_siso_alt(const R *sys, const R *par, R *ext, const size_t frame_id)
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

}
}
