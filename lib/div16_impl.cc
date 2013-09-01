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
#include "div16_impl.h"

namespace gr {
  namespace fmrds {

    div16::sptr
    div16::make()
    {
      return gnuradio::get_initial_sptr (new div16_impl());
    }

    /*
     * The private constructor
     */
    div16_impl::div16_impl()
      : gr_sync_block("div16",
		      gr_make_io_signature(2, 2, sizeof (float)),
		      gr_make_io_signature(1, 1, sizeof (float)))
    {
	set_history(2);

    d_state = 0.0;        // initial state is 0
    d_cntr = 0;           // crossings counter 0
    d_found_sync = 0;     // Sync is initially set to not found (0)

    }

    /*
     * Our virtual destructor.
     */
    div16_impl::~div16_impl()
    {
    }

    int
    div16_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const float *clk = (const float *) input_items[0];
        const float *data = (const float *) input_items[1];
        float *out = (float *) output_items[0];
	clk += 1; // ensure that i - 1 is valid.

	for (int i = 0; i < noutput_items; i++)
        {
                int sign_1 = 0;
                int sign_2 = 0;

                if(!d_found_sync && (clk[i] < clk[i-1]) && (data[i] < data[i-1]))
                {
                    printf("Clock syncd to data!")
                    d_found_sync = 1;
                }

                // Upward crossing
                if((clk[i] < clk[i-1]) && d_found_sync)
                {
                        d_cntr += 1;
                }

                if((__cntr == 8) && (__state == 0))
                {
                        d_state = 1.0;
                        d_cntr = 0;
                }
                else if ((__cntr == 8) && (__state == 1))
                {
                        d_state = 0.0;
                        d_cntr = 0;
                }

                out = &__state;
        }

        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (noutput_items);

	// Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace fmrds */
} /* namespace gr */

