#include <cmath>
#include <thread>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>

#include "Tools/general_utils.h"
#include "Tools/system_functions.h"
#include "Tools/Display/rang_format/rang_format.h"
#include "Tools/Display/Statistics/Statistics.hpp"
#include "Tools/Exception/exception.hpp"

#ifdef ENABLE_MPI
#include "Module/Monitor/Monitor_reduction_mpi.hpp"
#endif

#include "Factory/Module/Monitor/Monitor.hpp"
#include "Factory/Tools/Display/Terminal/Terminal.hpp"

#include "BFER.hpp"

using namespace aff3ct;
using namespace aff3ct::simulation;

template <typename B, typename R, typename Q>
BFER<B,R,Q>
::BFER(const factory::BFER::parameters& params_BFER)
: Simulation(params_BFER),
  params_BFER(params_BFER),

  barrier(params_BFER.n_threads),

  bit_rate((float)params_BFER.src->K / (float)params_BFER.cdc->N),
  noise(nullptr),

  monitor_mi    (params_BFER.n_threads, nullptr),
  monitor_mi_red(                       nullptr),
  monitor_er    (params_BFER.n_threads, nullptr),
  monitor_er_red(                       nullptr),

  dumper     (params_BFER.n_threads, nullptr),
  dumper_red (                       nullptr),

  rep_er   (nullptr),
  rep_mi   (nullptr),
  rep_noise(nullptr),
  rep_throughput(nullptr),
  terminal (nullptr),

  distributions(nullptr)
{
	if (params_BFER.n_threads < 1)
	{
		std::stringstream message;
		message << "'n_threads' has to be greater than 0 ('n_threads' = " << params_BFER.n_threads << ").";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	if (params_BFER.err_track_enable)
	{
		for (auto tid = 0; tid < params_BFER.n_threads; tid++)
			dumper[tid] = new tools::Dumper();

		std::vector<tools::Dumper*> dumpers;
		for (auto tid = 0; tid < params_BFER.n_threads; tid++)
			dumpers.push_back(dumper[tid]);

		dumper_red = new tools::Dumper_reduction(dumpers);
	}



	modules["monitor_er"] = std::vector<module::Module*>(params_BFER.n_threads, nullptr);
	for (auto tid = 0; tid < params_BFER.n_threads; tid++)
	{
		this->monitor_er[tid] = this->build_monitor_er(tid);
		modules["monitor_er"][tid] = this->monitor_er[tid];
	}

	#ifdef ENABLE_MPI
		// build a monitor to compute BER/FER (reduce the other monitors)
		this->monitor_er_red = new module::Monitor_reduction_mpi<Monitor_BFER_type>(this->monitor_er,
		                                                                            std::this_thread::get_id(),
		                                                                            params_BFER.mpi_comm_freq);
	#else
		// build a monitor to compute BER/FER (reduce the other monitors)
		this->monitor_er_red = new Monitor_BFER_reduction_type(this->monitor_er);
	#endif


	if (params_BFER.mutinfo)
	{
		modules["monitor_mi"] = std::vector<module::Module*>(params_BFER.n_threads, nullptr);
		for (auto tid = 0; tid < params_BFER.n_threads; tid++)
		{
			this->monitor_mi[tid] = this->build_monitor_mi(tid);
			modules["monitor_mi"][tid] = this->monitor_mi[tid];
		}

		#ifdef ENABLE_MPI
			// build a monitor to compute BER/FER (reduce the other monitors)
			this->monitor_mi_red = new module::Monitor_reduction_mpi<Monitor_MI_type>(this->monitor_mi,
			                                                                          std::this_thread::get_id(),
			                                                                          params_BFER.mpi_comm_freq);
		#else
			// build a monitor to compute BER/FER (reduce the other monitors)
			this->monitor_mi_red = new Monitor_MI_reduction_type(this->monitor_mi);
		#endif
	}

	if (!params_BFER.noise->pdf_path.empty())
		distributions = new tools::Distributions<R>(params_BFER.noise->pdf_path);



	this->noise = params_BFER.noise->template build<R>(0);

	rep_noise = new tools::Reporter_noise<R>(this->noise);
	reporters.push_back(rep_noise);

	if (params_BFER.mutinfo)
	{
		rep_mi = new tools::Reporter_MI<B,R>(*this->monitor_mi_red);
		reporters.push_back(rep_mi);
	}

	rep_er = new tools::Reporter_BFER<B>(*this->monitor_er_red);
	reporters.push_back(rep_er);

	rep_throughput = new tools::Reporter_throughput<uint64_t>(*this->monitor_er_red);
	reporters.push_back(rep_throughput);
}

template <typename B, typename R, typename Q>
BFER<B,R,Q>
::~BFER()
{
	release_objects();

	if (rep_noise != nullptr) { delete rep_noise; rep_noise = nullptr; }
	if (rep_er    != nullptr) { delete rep_er;    rep_er    = nullptr; }
	if (rep_mi    != nullptr) { delete rep_mi;    rep_mi    = nullptr; }

	if (monitor_mi_red != nullptr) { delete monitor_mi_red; monitor_mi_red = nullptr; }
	if (monitor_er_red != nullptr) { delete monitor_er_red; monitor_er_red = nullptr; }
	if (dumper_red     != nullptr) { delete dumper_red;     dumper_red     = nullptr; }

	for (auto tid = 0; tid < params_BFER.n_threads; tid++)
	{
		if (monitor_mi[tid] != nullptr) { delete monitor_mi[tid]; monitor_mi[tid] = nullptr; }
		if (monitor_er[tid] != nullptr) { delete monitor_er[tid]; monitor_er[tid] = nullptr; }
		if (dumper    [tid] != nullptr) { delete dumper    [tid]; dumper    [tid] = nullptr; }
	}

	if (terminal      != nullptr) { delete terminal;      terminal      = nullptr; }
	if (distributions != nullptr) { delete distributions; distributions = nullptr; }
	if (noise         != nullptr) { delete noise;         noise         = nullptr; }
}

template <typename B, typename R, typename Q>
void BFER<B,R,Q>
::_build_communication_chain()
{
	// build the communication chain in multi-threaded mode
	std::vector<std::thread> threads(params_BFER.n_threads -1);
	for (auto tid = 1; tid < params_BFER.n_threads; tid++)
		threads[tid -1] = std::thread(BFER<B,R,Q>::start_thread_build_comm_chain, this, tid);

	BFER<B,R,Q>::start_thread_build_comm_chain(this, 0);

	// join the slave threads with the master thread
	for (auto tid = 1; tid < params_BFER.n_threads; tid++)
		threads[tid -1].join();
}

template <typename B, typename R, typename Q>
void BFER<B,R,Q>
::launch()
{
	this->terminal = this->build_terminal();


	if (!this->params_BFER.err_track_revert)
	{
		this->build_communication_chain();

		if (tools::Terminal::is_over())
		{
			this->release_objects();
			return;
		}
	}

	int noise_begin = 0;
	int noise_end   = (int)params_BFER.noise->range.size();
	int noise_step  = 1;
	if (params_BFER.noise->type == "EP")
	{
		noise_begin = (int)params_BFER.noise->range.size()-1;
		noise_end   = -1;
		noise_step  = -1;
	}

	// for each NOISE to be simulated
	for (auto noise_idx = noise_begin; noise_idx != noise_end; noise_idx += noise_step)
	{
		if (this->noise != nullptr) delete noise;

		this->noise = params_BFER.noise->template build<R>(params_BFER.noise->range[noise_idx], bit_rate,
		                                                   params_BFER.mdm->bps, params_BFER.mdm->upf);

		// manage noise distributions to be sure it exists
		if (this->distributions != nullptr)
			this->distributions->read_distribution(this->noise->get_noise());


		if (this->params_BFER.err_track_revert)
		{
			this->release_objects();
			this->monitor_er_red->clear_callbacks();

			if (this->monitor_mi_red != nullptr)
				this->monitor_mi_red->clear_callbacks();

			// dirty hack to override simulation params_BFER
			auto &params_BFER_writable = const_cast<factory::BFER::parameters&>(params_BFER);

			std::stringstream s_noise;
			s_noise << std::setprecision(2) << std::fixed << this->noise->get_noise();

			params_BFER_writable.src     ->path = params_BFER.err_track_path + "_" + s_noise.str() + ".src";
			params_BFER_writable.cdc->enc->path = params_BFER.err_track_path + "_" + s_noise.str() + ".enc";
			params_BFER_writable.chn     ->path = params_BFER.err_track_path + "_" + s_noise.str() + ".chn";

			std::ifstream file(params_BFER.chn->path, std::ios::binary);
			if (file.is_open())
			{
				unsigned max_fra;
				file.read((char*)&max_fra, sizeof(max_fra));
				file.close();

				*const_cast<unsigned*>(&params_BFER.max_frame) = max_fra;
			}
			else
			{
				std::stringstream message;
				message << "Impossible to read the 'chn' file ('chn' = " << params_BFER.chn->path << ").";
				throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
			}

			this->build_communication_chain();

			if (tools::Terminal::is_over())
			{
				this->release_objects();
				return;
			}
		}

		if (params_BFER.display_legend)
		#ifdef ENABLE_MPI
			if (params_BFER.mpi_rank == 0)
		#endif
			if ((!params_BFER.ter->disabled && noise_idx == noise_begin && !params_BFER.debug)
				|| (params_BFER.statistics && !params_BFER.debug))
				terminal->legend(std::cout);

		// start the terminal to display BER/FER results
	#ifdef ENABLE_MPI
		if (params_BFER.mpi_rank == 0)
	#endif
		if (!params_BFER.ter->disabled && params_BFER.ter->frequency != std::chrono::nanoseconds(0) && !params_BFER.debug)
			terminal->start_temp_report(params_BFER.ter->frequency);

		try
		{
			this->_launch();
		}
		catch (std::exception const& e)
		{
			tools::Terminal::stop();
			terminal->final_report(std::cout); // display final report to not lost last line overwritten by the error messages

			rang::format_on_each_line(std::cerr, std::string(e.what()) + "\n", rang::tag::error);
			this->simu_error = true;
		}

	#ifdef ENABLE_MPI
		if (params_BFER.mpi_rank == 0)
	#endif
		if (!params_BFER.ter->disabled && terminal != nullptr && !this->simu_error)
		{
			if (params_BFER.debug)
				terminal->legend(std::cout);

			terminal->final_report(std::cout);

			if (params_BFER.statistics)
			{
				std::vector<std::vector<const module::Module*>> mod_vec;
				for (auto &vm : modules)
				{
					std::vector<const module::Module*> sub_mod_vec;
					for (auto *m : vm.second)
						sub_mod_vec.push_back(m);
					mod_vec.push_back(sub_mod_vec);
				}

				std::cout << "#" << std::endl;
				tools::Stats::show(mod_vec, true, std::cout);
				std::cout << "#" << std::endl;
			}
		}

		if (params_BFER.mnt_er->err_hist != -1)
		{
			auto err_hist = monitor_er_red->get_err_hist();

			if (err_hist.get_n_values() != 0)
			{
				std::string noise_value;
				switch (this->noise->get_type())
				{
					case tools::Noise_type::SIGMA:
						if (params_BFER.noise->type == "EBN0")
							noise_value = std::to_string(dynamic_cast<tools::Sigma<>*>(this->noise)->get_ebn0());
						else //(params_BFER.noise_type == "ESN0")
							noise_value = std::to_string(dynamic_cast<tools::Sigma<>*>(this->noise)->get_esn0());
						break;
					case tools::Noise_type::ROP:
					case tools::Noise_type::EP:
						noise_value = std::to_string(this->noise->get_noise());
						break;
				}

				std::ofstream file_err_hist(params_BFER.mnt_er->err_hist_path + "_" + noise_value + ".txt");
				file_err_hist << "\"Number of error bits per wrong frame\"; \"Histogram (noise: " << noise_value
				              << this->noise->get_unity() << ", on " << err_hist.get_n_values() << " frames)\"" << std::endl;

				int max;

				if (params_BFER.mnt_er->err_hist == 0)
					max = err_hist.get_hist_max();
				else
					max = params_BFER.mnt_er->err_hist;

				err_hist.dump(file_err_hist, 0, max, 0, false, false, false, "; ");
			}
		}

		if (this->dumper_red != nullptr && !this->simu_error)
		{
			std::stringstream s_noise;
			s_noise << std::setprecision(2) << std::fixed << this->noise->get_noise();

			this->dumper_red->dump(params_BFER.err_track_path + "_" + s_noise.str());
			this->dumper_red->clear();
		}

		if (!params_BFER.crit_nostop && !params_BFER.err_track_revert && !tools::Terminal::is_interrupt() &&
			!this->monitor_er_red->fe_limit_achieved() &&
		    (params_BFER.max_frame == 0 || this->monitor_er_red->get_n_analyzed_fra() >= params_BFER.max_frame))
			tools::Terminal::stop();

		if (tools::Terminal::is_over())
			break;

		this->monitor_er_red->reset();
		if (this->monitor_mi_red != nullptr)
			this->monitor_mi_red->reset();

		for (auto &m : modules)
			for (auto mm : m.second)
				if (mm != nullptr)
					for (auto &t : mm->tasks)
						t->reset_stats();

		tools::Terminal::reset();
	}

	this->release_objects();
}

template <typename B, typename R, typename Q>
void BFER<B,R,Q>
::release_objects()
{
}

template <typename B, typename R, typename Q>
module::Monitor_MI<B,R>* BFER<B,R,Q>
::build_monitor_mi(const int tid)
{
	return params_BFER.mnt_mi->build<B,R>();
}

template <typename B, typename R, typename Q>
module::Monitor_BFER<B>* BFER<B,R,Q>
::build_monitor_er(const int tid)
{
	bool count_unknown_values = params_BFER.noise->type == "EP";

	return params_BFER.mnt_er->build<B>(count_unknown_values);
}

template <typename B, typename R, typename Q>
tools::Terminal* BFER<B,R,Q>
::build_terminal()
{
	return params_BFER.ter->build(this->reporters);
}

template <typename B, typename R, typename Q>
void BFER<B,R,Q>
::start_thread_build_comm_chain(BFER<B,R,Q> *simu, const int tid)
{
	try
	{
		simu->__build_communication_chain(tid);

		if (simu->params_BFER.err_track_enable)
			simu->monitor_er[tid]->add_handler_fe(std::bind(&tools::Dumper::add, simu->dumper[tid], std::placeholders::_1, std::placeholders::_2));
	}
	catch (std::exception const& e)
	{
		tools::Terminal::stop();
		simu->simu_error = true;

		simu->mutex_exception.lock();

		auto save = tools::exception::no_backtrace;
		tools::exception::no_backtrace = true;
		std::string msg = e.what(); // get only the function signature
		tools::exception::no_backtrace = save;

		if (std::find(simu->prev_err_messages.begin(), simu->prev_err_messages.end(), msg) == simu->prev_err_messages.end())
		{
			// with backtrace if debug mode
			rang::format_on_each_line(std::cerr, std::string(e.what()) + "\n", rang::tag::error);

			simu->prev_err_messages.push_back(msg); // save only the function signature
		}
		simu->mutex_exception.unlock();
	}
}

// ==================================================================================== explicit template instantiation
#include "Tools/types.h"
#ifdef MULTI_PREC
template class aff3ct::simulation::BFER<B_8,R_8,Q_8>;
template class aff3ct::simulation::BFER<B_16,R_16,Q_16>;
template class aff3ct::simulation::BFER<B_32,R_32,Q_32>;
template class aff3ct::simulation::BFER<B_64,R_64,Q_64>;
#else
template class aff3ct::simulation::BFER<B,R,Q>;
#endif
// ==================================================================================== explicit template instantiation

