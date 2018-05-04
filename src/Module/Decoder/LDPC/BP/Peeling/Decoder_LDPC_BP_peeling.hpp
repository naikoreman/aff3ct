#ifndef DECODER_LDPC_BP_PEELING_HPP
#define DECODER_LDPC_BP_PEELING_HPP

#include "../Decoder_LDPC_BP.hpp"

namespace aff3ct
{
namespace module
{

template<typename B = int, typename R = float>
class Decoder_LDPC_BP_peeling : public Decoder_LDPC_BP<B,R>
{
protected:
	const bool multiframe_interleaving;

	const std::vector<unsigned> &info_bits_pos;

	// data structures for iterative decoding
	std::vector<std::vector<B>> var_nodes;
	std::vector<std::vector<B>> check_nodes;

public:
	Decoder_LDPC_BP_peeling(const int K, const int N, const int n_ite,
	                        const tools::Sparse_matrix &H,
	                        const std::vector<unsigned> &info_bits_pos,
	                        const bool enable_syndrome = true,
	                        const int syndrome_depth = 1,
	                        const bool multiframe_interleaving = false,
	                        const int n_frames = 1);
	virtual ~Decoder_LDPC_BP_peeling() = default;

protected:
	void _load          (const R *Y_N,         const int frame_id);

	void _store         (B *V_K,               const int frame_id);
	void _store_cw      (B *V_N,               const int frame_id);

	void _decode_hiho   (const B *Y_N, B *V_K, const int frame_id);
	void _decode_hiho_cw(const B *Y_N, B *V_N, const int frame_id);
	void _decode_siho   (const R *Y_N, B *V_K, const int frame_id);
	void _decode_siho_cw(const R *Y_N, B *V_N, const int frame_id);
	virtual void _decode(                      const int frame_id);

};

}
}

#endif //DECODER_LDPC_BP_PEELING_HPP