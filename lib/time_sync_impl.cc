/* -*- c++ -*- */
/* 
 * Copyright 2016 <+YOU OR YOUR COMPANY+>.
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
#include "time_sync_impl.h"

namespace gr {
  namespace fmrds {

    time_sync::sptr time_sync::make(const int n_samples, const int n_resync)
    {
      return gnuradio::get_initial_sptr(new time_sync_impl(n_samples, n_resync));
    }

    /*
     * The private constructor
     */
    time_sync_impl::time_sync_impl(const int n_samples, const int n_resync)
      : gr::sync_block("time_sync",
              gr::io_signature::make(2, 2, sizeof(float)),
              gr::io_signature::make(2, 2, sizeof(float)))
    {
      set_history(122);
      set_output_multiple(122);
      
      // Sync machine variables
      d_offset = 0;
      d_syncd = 0;
      d_half_cycle_cntr = 0;
      d_sample_cntr = 0;
      d_resync_cntr = 0;
      d_resync_timeout = n_resync;
      d_n_sampl = n_samples;
      d_clock_pos = 0;

      // Initializing the data and pos vectors
      for (int i = 0; i < 112; i++) d_data[i] = 0.0;
      for (int i = 0; i < 100; i++) d_pos[i] = 0;
    }

    /*
     * Our virtual destructor.
     */
    time_sync_impl::~time_sync_impl()
    {
    }

    /*
     * The theory behind the time synchronization is as follows:
     * 
     * We want to align half a cycle of the RDS signal into half a cycle of the clock. 
     * To do that, we want to extract the arg_max of the absolute value of RDS signal 
     * (since it varies from positive to negative). The arg_max of the absolute value
     * of RDS signal is the position where the maximum occurs ranging from (0 to 112    
     * for a 266 kHz sampling rate). The arg_max will then point to the position where    
     * the *half clock cycle* should be. We then delay the clock by enough samples to     
     * have half of its cycle aligned to the arg_max. We then output both the data and  
     * the re-synce'd clock.
     *
     * Since the biphase signal can be changing phase when we try to syncronize (i.e.,  
     * two consecutive maxima or minima), best is to evaluate the arg_max over several 
     * clock cycles (N_data_cycles), to reduce the possibility of having phase changes
     * during the arg_max estimation.
     *
     * The implementation:
     *
     * A vector of 112_samples is fed with the absolute values of incoming samples
     * until its full.
     *
     * We then calculate the arg_max (position of the max) for the vector and we take
     * the average of the arg_maxs over N half cycles in order to smooth out errors. 
     * This is then considered to be the position of half a cycle of the clock.
     * 
     * THIS EXPLANATION IS MOSTLY UP-TO-DATE BUT NOT ENTIRELY
     * THE SYNC ALGORITH STILL PRODUCES LESS THAN PERFECT RESULTS
     * PROBLEM SEEMS TO BE IN A PHASE DRIFT OF THE RDS SIGNAL WRT THE CLOCK
     * MAYBE THE REAL RDS BITRATE IS NOT GOOD...
     */
    int time_sync_impl::work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];
        const float *clk = (const float *) input_items[1];
        float *out = (float *) output_items[0];
        float *s_clk = (float *) output_items[1];

        in += 112; // ensure that i - 1 is valid.
        clk += 112;

        for (int i = 0; i < noutput_items; i++)
        { 
          if (d_syncd == 0) // Not syncd, but searching for a sync
          {
            if(clk[i] != clk[i-1])
            {
              d_clock_pos = d_sample_cntr;
            }

            if (d_sample_cntr < 112)
            {
              // Not enough samples, continue filling in the vector
              d_data[d_sample_cntr] = std::abs(in[i]);
              d_sample_cntr++;
            }
            else
            {
              // Search for the position of the maximum
              int temp_max = d_data[0];
              int temp_pos = 0;
            
              for (int i = 1; i < 112; i++)
              {
                if (d_data[i] >= temp_max)
                {
                  temp_max = d_data[i];
                  temp_pos = i;
                }
              }

              d_pos[d_half_cycle_cntr] = temp_pos;

              d_sample_cntr = 0;
              d_half_cycle_cntr++;
            }

            // If we have enough estimations of positions then we can 
            // calculate the average:
            float avg_pos = 0.0;

            if (d_half_cycle_cntr >= d_n_sampl)
            {
              for (int i = 0; i < d_n_sampl; i++) avg_pos += (float)d_pos[i];
              avg_pos /= d_n_sampl;

              if (std::abs(int(avg_pos) - d_clock_pos) > 56)
              {
                d_offset = 112 - d_clock_pos - int(avg_pos);
              }
              else
              {
                d_offset = 56 + d_clock_pos - int(avg_pos);
              }
              

              printf("syncd with offset: %d clock: %d\n", d_offset, d_clock_pos);
              
              d_syncd = 1;
              
              d_half_cycle_cntr = 0;
              
            }
          }
          else
          {
            d_resync_cntr++;

            // Resync from time to time
            if (d_resync_cntr > d_resync_timeout)
            {
              d_syncd = 0;
              d_resync_cntr = 0;
            }
          }

          out[i] = std::abs(in[i]);;
          s_clk[i] = clk[(i - d_offset)];
        } 

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }
  } /* namespace fmrds */
} /* namespace gr */

