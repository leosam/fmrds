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

							 // Initialization of the precalculated syndromes for the check words
		int checks[50] = 	{0,0,1,1,1,1,1,1,0,0,  // check word A
							 0,1,1,0,0,1,1,0,0,0,  // check word B
							 0,1,0,1,1,0,1,0,0,0,  // check word C
							 1,1,0,1,0,1,0,0,0,0,  // check word C'
							 0,1,1,0,1,1,0,1,0,0}; // check word D

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

    	// Initializing the check words
    	for (int i = 0; i < 50; i++)
    	{
    		d_checks[i] = checks[i];
    	}

    	d_last_syndrome = -1;
     	d_last_seen = 0;
	    d_sync_cntr = 0;
    	d_overall = 0;
    	d_syncd = 0;

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

		int res[10] = {0,0,0,0,0,0,0,0,0,0};
		int input_seq[26] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		int tmp = 0, val = 0;


		for (int i = 0; i < noutput_items; i++)
		{

			// Isolate the input sequence
			for (int x = 0; x < 26; x++)
			{
				input_seq[x] = in[i+x];  // Data from 0 to 15, check from 16 to 25
			}

			// if (!d_syncd)
			// {
				// Calculate the syndrome for the current input sequence
				syndrome_calc(&input_seq[0], &res[0]);
				
				// Check against known syndromes
				for (int x = 0; x < 5; x++)
				{
					if (memcmp(res, &d_syndromes[x * 10], sizeof(res)) == 0)
					{
						// We're in presync!

						// Check if we're sync'ed
						if (d_sync_cntr == 26)
						{
							if ((x == (d_last_syndrome + 1)) || ((x == 4) && (d_last_syndrome == 2 )) || ((x == 3) && (d_last_syndrome == 1 )) || ((x == 0) && (d_last_syndrome == 4 )))
							{
								d_syncd = 1;
							}
							else
							{
								d_syncd = 0;
							}
						}
						else
						{
							d_syncd = 0;
						} 

						// DEBUG CODE:
						if (d_syncd == 1)
						{
							printf(" [ ");
							for (int k = 0; k < 26; k++)
							{
								printf("%d ", input_seq[k]);
							}	
							printf("] %d <- %d\n", x, d_last_syndrome);
						}

						d_last_syndrome = x;
						d_sync_cntr = 0;

						break;
					}
				}

				// lost presync
				if (d_sync_cntr > 26)
				{
					d_last_syndrome = -1;
					//d_syncd = 0;
				}

			// }
			// else
			// {
			// 	// Check if we're sync'ed
			// 	if (d_sync_cntr == 26)
			// 	{
			//		// TODO: We need to do this for the NEXT block, so we need to know which one it is!
			// 		for (int x = 0; x < 10; x++)
			// 		{
			// 			// Remove the check word
			// 			input_seq[16+x] = input_seq[16+x] ^ d_checks[(d_last_syndrome * 10)+x];

			// 			// Calculate the syndrome for the current input sequence
			// 			syndrome_calc(&input_seq[0], &res[0]);
			// 		}

			// 		tmp = 0;

			// 		for (int x = 0; x < 10; x++)
			// 		{
			// 			tmp += res[x];
			// 		}

			// 		// If the syndrome equals 0 it means we have a zero error block
			// 		if (tmp == 0)
			// 		{
			// 			printf(" [ ");
			// 			for (int k = 0; k < 26; k++)
			// 			{
			// 				printf("%d ", input_seq[k]);
			// 			}	
			// 			printf("] %d <- %d\n", x);
			// 		}
			// 		else
			// 		{
			// 			d_last_syndrome = -1;
			// 			d_sync_cntr = 0;
			// 		}

			// 	}

			//}

			d_sync_cntr++;

			out[i] = in[i];
			syncd[i] = d_last_syndrome;

		}

        consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

    void sync_impl::syndrome_calc(const int* in, int* res)
    {
    	int tmp = 0;

    	// Syndrome matrix multiplication
		for (int y = 0; y < 10; y++)
		{
			tmp = 0;

			for (int x = 0; x < 26; x++)
			{
				tmp += d_parity_chk[x][y] * in[x];
			}

			res[y] = tmp % 2;
		}
    }

  } /* namespace fmrds */
} /* namespace gr */

