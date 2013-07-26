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
#include "sync_impl.h"

namespace gr {
  namespace fmrds {

	// Initialization of the parity check matrix
	//     This is used for syndrome calculation
	//     and syncronization
	const char __parity_chk[26][10] = 
		                     {{1,0,0,0,0,0,0,0,0,0},
					  		  {0,1,0,0,0,0,0,0,0,0},
							  {0,0,1,0,0,0,0,0,0,0},
							  {0,0,0,1,0,0,0,0,0,0},
							  {0,0,0,0,1,0,0,0,0,0},
							  {0,0,0,0,0,1,0,0,0,0},
							  {0,0,0,0,0,0,1,0,0,0},
							  {0,0,0,0,0,0,0,1,0,0},
							  {0,0,0,0,0,0,0,0,1,0},
							  {0,0,0,0,0,0,0,0,0,1},
							  {1,0,1,1,0,1,1,1,0,0},
							  {0,1,0,1,1,0,1,1,1,0},
							  {0,0,1,0,1,1,0,1,1,1},
							  {1,0,1,0,0,0,0,1,1,1},
							  {1,1,1,0,0,1,1,1,1,1},
							  {1,1,0,0,0,1,0,0,1,1},
							  {1,1,0,1,0,1,0,1,0,1},
							  {1,1,0,1,1,1,0,1,1,0},
							  {0,1,1,0,1,1,1,0,1,1},
							  {1,0,0,0,0,0,0,0,0,1},
							  {1,1,1,1,0,1,1,1,0,0},
							  {0,1,1,1,1,0,1,1,1,0},
							  {0,0,1,1,1,1,0,1,1,1},
							  {1,0,1,0,1,0,0,1,1,1},
							  {1,1,1,0,0,0,1,1,1,1},
							  {1,1,0,0,0,1,1,0,1,1}};

    // Initialization of the precalculated syndromes for the check words
	const char __syndromes[5][10] = 
		                     {{1,1,1,1,0,1,1,0,0,0},  // check word A
					  		  {1,1,1,1,0,1,0,1,0,0},  // check word B
							  {1,0,0,1,0,1,1,1,0,0},  // check word C
							  {1,1,1,1,0,0,1,1,0,0},  // check word C'
							  {1,0,0,1,0,1,1,0,0,0}}; // check word D						  
	
	// Last identified syndrome
    //      0->A, 1->B, 2->C, 3->C', 4->D
	//     -1->"no sync"						  
	char __last_syndrome = -1;

	// Last identified syndrome
    //      occurence
	char __last_seen = -1;
	
    sync::sptr
    sync::make()
    {
      return gnuradio::get_initial_sptr (new sync_impl());
    }

    /*
     * The private constructor
     */
    sync_impl::sync_impl()
      : gr_block("sync",
		      gr_make_io_signature(1, 1, sizeof (char)),
		      gr_make_io_signature(2, 2, sizeof (char)))
    {
		// Size of a group
    	set_history(26);
		
    }

    /*
     * Our virtual destructor.
     */
    sync_impl::~sync_impl()
    {
    }

    void
    sync_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int sync_impl::general_work (int noutput_items, gr_vector_int &ninput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items)
    {
        const char *in = (const char *) input_items[0];
        char *out = (char *) output_items[0];
		char *syncd = (char *) output_items[1];

		// Sync'd flag
		char sync = 0;

		in += 25; // ensure that i - 1 is valid.

		for (int i = 0; i < noutput_items; i+=26)
		{
			char res[10] = {0,0,0,0,0,0,0,0,0,0};
			char cmp[5] = {0,0,0,0,0};
		
			// Matrix multiplication
			for(int y = 0; y < 10; y++)
			{
				for(int x = 0; x < 26; x++)
				{
					res[y] += in[i-x] * __parity_chk[x][y];
				}
				
				// All operation are modulo 2 (bits): apply a mask to 1
				res[y] = res[y] & 1;
			}
			
			// Check against known syndromes

			for(int x = 0; x < 5; x++)
			{
				sync = x;

				for(int y = 0; y < 10; y++)
				{
					if(res[y] != __syndromes[x][y])
					{
						sync = 0;
						break;
					}
				}
			}

			// Pre-initialize with none, in case the next block doesnt capture any sync
			//__last_syndrome = -1;

			// Decrement last seen
			//__last_seen -= 1;

			//for(int x = 0; x < 5; x++)
			//{
			//	if(cmp[x])
			//	{
			//		__last_syndrome = x;
			//		__last_seen = 26;
			//		syncd[i] = x;
			//	}
			//}

			// Tagged passthrough the input to the output
			if(sync)
			{
				out[i] = in[i] & 2;
			}
			else
			{
				out[i] = in[i] & 1;
			}
			
		}

		
		
        // Do <+signal processing+>
        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace fmrds */
} /* namespace gr */

