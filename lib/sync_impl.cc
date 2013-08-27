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

#include <stdio.h>
#include <string.h>

namespace gr {
  namespace fmrds {		  
	
    sync::sptr sync::make()
    {
      return gnuradio::get_initial_sptr (new sync_impl());
    }

    /*
     * The private constructor
     */
    sync_impl::sync_impl() : gr_block("sync", gr_make_io_signature(1, 1, sizeof (char)), gr_make_io_signature(2, 2, sizeof (char)))
    {
		// Size of a group
    	set_history(26);
    	//set_output_multiple(26);

    	// Pparity check matrix
		//     This is used for syndrome calculation
		//     and syncronization
    	int pchk[26][10] =  {{1,0,0,0,0,0,0,0,0,0},
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
		int syns[50] = 		{1,1,1,1,0,1,1,0,0,0,  // check word A
							 1,1,1,1,0,1,0,1,0,0,  // check word B
							 1,0,0,1,0,1,1,1,0,0,  // check word C
							 1,1,1,1,0,0,1,1,0,0,  // check word C'
							 1,0,0,1,0,1,1,0,0,0}; // check word D

    	// Initializing the property matrices
    	for (int i = 0; i < 26; i++)
    	{
    		for (int j = 0; j < 10; j++)
    		{
    			d_parity_chk[i][j] = pchk[i][j];
    		}
    	}

    	// Initializing the property matrices
    	for (int i = 0; i < 50; i++)
    	{
    		d_syndromes[i] = syns[i];
    	}

    	d_last_syndrome = -1;
     	d_last_seen = 0;
	    d_sync_cntr = 0;
    	d_overall = 0;

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
        ninput_items_required[0] = noutput_items;
    }



    int sync_impl::general_work (int noutput_items, gr_vector_int &ninput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items)
    {
        const char *in = (const char *) input_items[0];
        char *out = (char *) output_items[0];
		char *syncd = (char *) output_items[1];

		//if (d_sync_cntr != 0)
		//printf(" %d ", noutput_items);

		//d_overall++;

		//printf("= %d, %d\n", in[0], in[1]);
		int res[10] = {0,0,0,0,0,0,0,0,0,0};

		int tmp = 0;

		int val = 0;
		

		for (int i = 0; i < noutput_items; i++)
		{
			//memset(res, 0, sizeof(res));

			// TODO: problem with time delay in sync marking. (always setting the sync at the last bit) 
			// Matrix multiplication
			for (int y = 0; y < 10; y++)
			{
				tmp = 0;

				for (int x = 0; x < 26; x++)
				{
					tmp += d_parity_chk[x][y] * in[i+x];
				}

				res[y] = tmp % 2;
			}

			// All operation are modulo 2 (bits): apply a mask to 1
			//for (int y = 0; y < 10; y++)
			//{
			//	res[y] = res[y] & (char)1;
			//}
			
			// Check against known syndromes
			for (int x = 0; x < 5; x++)
			{
				if (memcmp(res, &d_syndromes[x * 10], sizeof(res)) == 0)
				{
					if (x == 0)
					{
						printf(" [");

						for (int k = 0; k < 26; k++)
						{
							printf("%d ",in[i+k]);
						}	
						printf("] ");
					}

					if (x == 0)
					{
						printf("[");
						for (int k = 0; k < 10; k++)
						{
							printf("%d ", res[k]);
						}	
						printf("] %d\n", x);
					}

					d_last_syndrome = x;
					//d_sync_cntr = 25;
					//d_last_seen = d_overall;
					//break;
				}
			}

			// We're not (or no longer) synchronized, so lets search for sync
			//if (d_sync_cntr > 0)
			//{
			//	// We're still synchronized, so we increment the counter and leave
			//	d_sync_cntr--;
			//}
			//else
			//{
			//	d_last_syndrome = -1;
			//}

			out[i] = in[i];
			syncd[i] = d_last_syndrome;

		}

		//printf("-- %d, %d, %d", out[noutput_items-1], out[noutput_items], out[noutput_items+1]);
		
        // Do <+signal processing+>
        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace fmrds */
} /* namespace gr */

