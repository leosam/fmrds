/* -*- c++ -*- */
/* 
 * Copyright 2013 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gr_io_signature.h>
#include "biphase_decoder_impl.h"

namespace gr {
  namespace fmrds {

    float __out_bit = 0.0;

    biphase_decoder::sptr
    biphase_decoder::make()
    {
      return gnuradio::get_initial_sptr (new biphase_decoder_impl());
    }

    /*
     * The private constructor
     */
    biphase_decoder_impl::biphase_decoder_impl()
      : gr_sync_block("biphase_decoder",
		      gr_make_io_signature(2, 2, sizeof (float)),
		      gr_make_io_signature(1, 1, sizeof (float)))
    { 
	set_history(2);
    }

    /*
     * Our virtual destructor.
     */
    biphase_decoder_impl::~biphase_decoder_impl()
    {
    
	}

    int biphase_decoder_impl::work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];
        const float *clk = (const float *) input_items[1];
        float *out = (float *) output_items[0];

		in += 1; // ensure that i - 1 is valid.
		clk += 1;

        for (int i = 0; i < noutput_items; i++)
		{

			// In the falling edge of the clock
			if(clk[i] < clk[i-1])
			{
			
				// Check in which edge of the data we're in
				// to reconstruct the original BPSK signal
				if(in[i] < in[i-1])
				{
					// Falling edge: 1
					__out_bit = 1.0;
				}
				else
				{
					// Rising edge: 0
					__out_bit = -1.0;
				}
			}

			out[i] = __out_bit;
        } 

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace fmrds */
} /* namespace gr */

