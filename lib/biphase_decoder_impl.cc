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

#include <gnuradio/io_signature.h>
#include "biphase_decoder_impl.h"

namespace gr {
  namespace fmrds {

    biphase_decoder::sptr
    biphase_decoder::make()
    {
      return gnuradio::get_initial_sptr (new biphase_decoder_impl());
    }

    /*
     * The private constructor
     */
    biphase_decoder_impl::biphase_decoder_impl() : gr::sync_block("biphase_decoder", gr::io_signature::make(2, 2, sizeof (float)), gr::io_signature::make(2, 2, sizeof (float)))
    {
      set_history(244);         // To accomodate for clock sync slips
      
      d_out_bit = 0.0;    // Initial state is 0

      // Sync machine variables
      d_offset = 0;
      d_offset_cntr = 0;
      d_syncd = -1;
      d_resync_cntr = 0;
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
        float *s_clk = (float *) output_items[1];

		    in += 244; // ensure that i - 1 is valid.
		    clk += 244;

        for (int i = 0; i < noutput_items; i++)
		    {

          // Update the offset timer
          if (d_syncd == 0) d_offset_cntr++;

          // Detect an upward zero crossing
          if ((d_syncd == 0) && (sgn(in[i]) > sgn(in[i-10])))
          {
            d_syncd = 1;
            printf(".\n");
          }

          // Sync offset timer stop at a downward zero crossing
          if ((d_syncd == 1) && (sgn(in[i]) < sgn(in[i-10])))
          {
            d_offset = d_offset_cntr;
            d_syncd = 2;
            printf("Sync offset of %d\n", d_offset);
          }

          // In the falling edge of the clock
          if(clk[(i - d_offset)] < clk[(i - d_offset)-1])
          {
            if (d_syncd < 0)
            {
              d_offset_cntr = 0;
              d_syncd = 0;       // Start offset timer start
            }

            // Check in which edge of the data we're in
				    // to reconstruct the original BPSK signal
				    if (d_syncd == 2)
            {
              if(in[i] < in[i-10])
				      {
                // Falling edge: 1
                d_out_bit = 1.0;
				      }
				      else
				      {
                // Rising edge: 0
                d_out_bit = -1.0;
				      }
            }
            
          }

          // If the times goes off before we get a sync, restart...
          if (d_offset_cntr > 243) d_syncd = -1;

          // Every number of input samples, resync
          /*if (++d_resync_cntr == 10000000)
          {
            d_resync_cntr = 0;
            d_offset_cntr = 0;
            d_syncd = -1;
          }*/

          // Every number of input samples, resync
          /*if (++d_resync_cntr == 244*10000)
          {
            d_resync_cntr = 0;

            if ((sgn(in[i]) > sgn(in[i-1])))
            {
              d_offset = d_offset_cntr;
              d_syncd = 1;
              printf("Resync offset of %d\n", d_offset);
            }
          }*/

          out[i] = d_out_bit;
          s_clk[i] = clk[(i - d_offset)];
        } 

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

    int biphase_decoder_impl::sgn(float val)
    {
        return (float(0) < val) - (val < float(0));
    }

  } /* namespace fmrds */
} /* namespace gr */

