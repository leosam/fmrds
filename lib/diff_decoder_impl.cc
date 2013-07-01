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
#include "diff_decoder_impl.h"

namespace gr {
  namespace fmrds {

    diff_decoder::sptr
    diff_decoder::make()
    {
      return gnuradio::get_initial_sptr (new diff_decoder_impl());
    }

    /*
     * The private constructor
     */
    diff_decoder_impl::diff_decoder_impl()
      : gr_sync_block("diff_decoder", gr_make_io_signature(1, 1, sizeof (char)), gr_make_io_signature(1, 1, sizeof (char)))
    {
    	set_history(2);
    }

    /*
     * Our virtual destructor.
     */
    diff_decoder_impl::~diff_decoder_impl()
    {
    }

    int diff_decoder_impl::work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items)
    {
        const char *in = (const char *) input_items[0];
        char *out = (char *) output_items[0];

		in += 1; // ensure that i - 1 is valid.

		for (int i = 0; i < noutput_items; i++)
        {
			// A bad XOR, but works...
			if( in[i] == in[i-1] )
			{
				out[i] = 0;
			}
			else
			{
				out[i] = 1;
			}
        }

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace fmrds */
} /* namespace gr */

