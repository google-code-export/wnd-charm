#ifndef __ALGORITHM_TIMING_H__
#define __ALGORITHM_TIMING_H__

#include <map>
#include <string>
#include <sstream>
#include <ctime>
#include <cfloat> // Has definition of DBL_MAX
#include <cmath> // Has definition of DBL_MAX
/*
Example:
#include "AlgorithmTiming.h"

	// The object is a singleton, so it can be used to collect statistics
	// anywhere in the code-base.
	AlgorithmTiming &myFT = AlgorithmTiming::Instance();

	// Start a timer, with an algorithm name, and a parameter
	// that affects the algorithm run-time (e.g. the N in O = N log(N)).
	// Separate statistics will be kept for each algorithm-parameter combination
	AlgorithmTimeRef time1 = myFT.start("Foo",200);
	// Call the Foo algorithm to let it run
	myFT.stop(time1);

	AlgorithmTimeRef time2 = myFT.start("Foo2",300);
	// Call the Foo2 algorithm to let it run
	myFT.stop(time2);
	// Take another sample for Foo2 with "300".
	time2 = myFT.start("Foo2",300);
	// Call the Foo2 algorithm to let it run again
	myFT.stop(time2);
	
	myFT.report();
	prints a report to stdout.  The run-times are in milliseconds of CPU time (not elapsed time):
	Name   param  n samp  min  max  mean  stddev
	Foo    200       1     12   12   12      0
	Foo2   300       2     12   12   12      0



*/
typedef struct {
	std::string name;
	unsigned long npix;
	clock_t start;
	bool running;
	double min, max;
	double old_mean;
	double mean;
	double old_sum_var;
	double sum_var;
	unsigned long nsamples;
} AlgorithmTime;
typedef AlgorithmTime &AlgorithmTimeRef;
typedef std::map<std::string, std::map<unsigned long, AlgorithmTime> > AlgorithmTimings_T;

class AlgorithmTiming {
public:

	static AlgorithmTiming& Instance() {
		static AlgorithmTiming theSingleton;
		return theSingleton;
	}

	AlgorithmTimeRef start (std::string alg_name, unsigned long npix) {
		AlgorithmTime FG_time = {
			alg_name,
			npix,
			0,
			true,
			DBL_MAX, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0
		};

		AlgorithmTimings_T::iterator AT_map_it = AlgorithmTimings.find (alg_name);

		if (AT_map_it != AlgorithmTimings.end()) {
			std::map<unsigned long, AlgorithmTime>::iterator AP_map_it = AT_map_it->second.find (npix);
			if (AP_map_it == AT_map_it->second.end()) {
				AlgorithmTimings[alg_name][npix] = FG_time;
			}
		} else {
			AlgorithmTimings[alg_name][npix] = FG_time;
		}
		AlgorithmTimings[alg_name][npix].running = true;
		AlgorithmTimings[alg_name][npix].start = clock();
		return (AlgorithmTimings[alg_name][npix]);
	}
	
	void stop (AlgorithmTimeRef alg_timer) {
		struct timeval stoptime;
		clock_t t_stop = clock();
		double delta_t;

		if (! alg_timer.running) return;
		
	// Compute the time difference
		delta_t = (double) (t_stop - alg_timer.start) / (double) (CLOCKS_PER_SEC / 1000.0);

	// Compute a running mean and variance
	// var = alg_timer.sum_var / (alg_timer.nsamples - 1)
	// std_dev = sqrt (var);
	// See Knuth TAOCP vol 2, 3rd edition, page 232
		alg_timer.nsamples++;
		if (alg_timer.nsamples == 1) {
			alg_timer.old_mean = alg_timer.mean = delta_t;
			alg_timer.old_sum_var = 0.0;
		} else {
			alg_timer.mean = alg_timer.old_mean + (delta_t - alg_timer.old_mean)/alg_timer.nsamples;
			alg_timer.sum_var = alg_timer.old_sum_var + (delta_t - alg_timer.old_mean)*(delta_t - alg_timer.mean);
			
			// set up for next iteration
			alg_timer.old_mean = alg_timer.mean; 
			alg_timer.old_sum_var = alg_timer.sum_var;
		}

		if (delta_t < alg_timer.min) alg_timer.min = delta_t;
		if (delta_t > alg_timer.max) alg_timer.max = delta_t;
		alg_timer.running = false;
	}
	
	void report () {
		AlgorithmTimings_T::iterator AT_map_it = AlgorithmTimings.begin ();
		printf ("Name\tparam\tn samp\tmin\tmax\tmean\tstddev\n");
		while (AT_map_it != AlgorithmTimings.end()) {
			std::map<unsigned long, AlgorithmTime>::iterator AP_map_it = AT_map_it->second.begin();
			while (AP_map_it != AT_map_it->second.end()) {
				printf ("%s\t%lu\t%lu\t%.6g\t%.6g\t%.6g\t%.6g\n",AP_map_it->second.name.c_str(), AP_map_it->second.npix,
					AP_map_it->second.nsamples, AP_map_it->second.min, AP_map_it->second.max,
					AP_map_it->second.nsamples > 0 ? AP_map_it->second.mean : 0.0,
					AP_map_it->second.nsamples > 1 ? sqrt ( AP_map_it->second.sum_var / (AP_map_it->second.nsamples - 1)) : 0.0
				);
				AP_map_it++;
			}
			AT_map_it++;
		}
	}
	
	bool SSE2Supported() {
		try {
#ifdef __IA64__
#define TRY_SSE2 1
#elif __i386__
#define TRY_SSE2 1
#else
#define TRY_SSE2 0
#endif

#ifdef TRY_SSE2
			asm (
	//    		".intel_syntax noprefix\n"
				"xorpd %xmm0, %xmm0\n"
				"xor %eax, %eax\n"
			);
			return true;
#else
			return false;
#endif
	
	   } catch(...) {
		   return false; // xorpd not supported
	   }
	}



	std::string getGCCvers() {
#ifdef __GNUC__
		std::stringstream vers;
		vers << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__;
		return vers.str();
#else
		return ("");
#endif
	}
	
	bool EigenVectorized() {
#ifdef EIGEN_VECTORIZE
		return true;
#else
		return false;
#endif
	}



private:
	AlgorithmTiming() {}; // ctor hidden
	AlgorithmTiming(AlgorithmTiming const&) {}; // copy ctor hidden
	AlgorithmTiming& operator=(AlgorithmTiming const&) {}; // assign op. hidden
	~AlgorithmTiming() {}; // dtor hidden
	static AlgorithmTimings_T AlgorithmTimings;
};
#endif // __ALGORITHM_TIMING_H__


