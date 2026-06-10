// Allpass filter declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _allpass_
#define _allpass_

#if defined(__wasm__)
#include <cmath>
#endif

class allpass
{
public:
					allpass();
			void	setbuffer(float *buf, int size);
	inline  float	process(float inp);
			void	mute();
			void	setfeedback(float val);
			float	getfeedback();
// private:
	float	feedback;
	float	*buffer;
	int		bufsize;
	int		bufidx;
};


// Big to inline - but crucial for speed

inline float allpass::process(float input)
{
	float bufout = buffer[bufidx];
#if defined(__wasm__)
	// No FTZ/DAZ on wasm (see denormals.h): flush the recirculating sample
	// manually so denormals don't make the reverb crawl.
	if (std::fabs(bufout) < 1e-15f) bufout = 0.0f;
#endif
	buffer[bufidx] = input + (bufout*feedback);

	if(++bufidx>=bufsize) bufidx = 0;

	return -input + bufout;
}

#endif//_allpass

//ends