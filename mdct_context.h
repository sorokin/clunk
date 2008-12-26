#ifndef MDCT_CONTEXT_H__
#define MDCT_CONTEXT_H__

#include "fft_context.h"
#include <string.h>

namespace clunk {

template<int N, typename T>
struct window_func_base {
	virtual ~window_func_base() {}
	virtual T operator() (int x) const = 0;
	void precalculate() {
		for(int i = 0; i < N; ++i) {
			cache[i] = (*this)(i);
		}
	}
	T cache[N];
};


template<int BITS, template <int, typename> class window_func_type , typename T = float> 
class mdct_context {

	typedef fft_context<BITS - 2, T> fft_type;
	fft_type fft;

public: 
	enum { N = 1 << BITS , M = N / 2, N4 =  fft_type::N };
	
	typedef T value_type;
	typedef std::complex<T> complex_type;

	T data[N];
	
	mdct_context() {
		window_func.precalculate();
	}
	
	void mdct(bool inversion) {
		assert(N / 4 == N4); //we need static_assert :(
		if (!inversion) {
			T rotate[N];
			unsigned int t;
			for(t = 0; t < N4; ++t) {
				rotate[t] = -data[t + 3 * N4];
			}
			for(; t < N; ++t) {
				rotate[t] = data[t - N4];
			}
			for(t = 0; t < N4; ++t) {
				T re = (rotate[t * 2] - rotate[N - 1 - t * 2]) / 2;
				T im = (rotate[M + t * 2] - rotate[M - 1 - t * 2]) / -2;
				
				std::complex<T> a = std::polar<T>(1, 2 * T(M_PI) * (t + T(0.125)) / N);
				fft.data[t] = std::complex<T>(re * a.real() + im * a.imag(), -re * a.imag() + im * a.real());
			}
			fft.fft(false);
			T sqrt_N = sqrt((T)N);

			for(t = 0; t < N4; ++t) {
				std::complex<T> a = std::polar<T>(1, 2 * T(M_PI) * (t + T(0.125)) / N);
				std::complex<T>& f = fft.data[t];
				f = std::complex<T>(2 / sqrt_N * (f.real() * a.real() + f.imag() * a.imag()), 2 / sqrt_N * (-f.real() * a.imag() + f.imag() * a.real()));
			}

			for(t = 0; t < N4; ++t) {
				data[2 * t] = fft.data[t].real();
				data[M - 2 * t - 1] = -fft.data[t].imag();
			}
		} else {
			unsigned int t; 
			for(t = 0; t < N4; ++t) {
				T re = data[t * 2] / 2, im = data[M - 1 - t * 2] / 2;
				std::complex<T> a = std::polar<T>(1, 2 * T(M_PI) * (t + T(0.125)) / N);
				fft.data[t] = std::complex<T>(re * a.real() + im * a.imag(), - re * a.imag() + im * a.real());
			}
			fft.fft(false);
			T sqrt_N = sqrt((T)N);
			
			for(t = 0; t < N4; ++t) {
				std::complex<T> a = std::polar<T>(1, 2 * T(M_PI) * (t + T(0.125)) / N);
				std::complex<T>& f = fft.data[t];
				fft.data[t] = std::complex<T>(8 / sqrt_N * (f.real() * a.real() + f.imag() * a.imag()), 8 / sqrt_N * (-f.real() * a.imag() + f.imag() * a.real()));
			}

			T rotate[N];
			for(t = 0; t < N4; ++t) {
				rotate[2 * t] = fft.data[t].real();
				rotate[M + 2 * t] = fft.data[t].imag();
			}
			for(t = 1; t < N; t += 2) {
				rotate[t] = - rotate[N - t - 1];
			}

			//shift
			for(t = 0; t < 3 * N4; ++t) {
				data[t] = rotate[t + N4];
			}
			for(; t < N; ++t) {
				data[t] = -rotate[t - 3 * N4];
			}
		}
	}

	
	void apply() {
		for(unsigned i = 0; i < N; ++i) {
			data[i] *= window_func.cache[i];
		}
	}
	
	void clear() {
		memset(data, 0, sizeof(data));
	}
	
private:
	window_func_type<N, T> window_func;
	
	inline T result(unsigned idx) const {
		int sign;
		if (idx & 1) {
			idx = N - idx - 1;
			sign = -1;
		} else 
			sign = 1;
		
		return sign * (
			(idx < N / 2)?
				fft.data[idx / 2].real():
				fft.data[(idx - N / 2) / 2].imag()
		);
	}
};

}

#endif
