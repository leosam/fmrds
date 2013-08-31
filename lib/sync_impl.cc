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

		int synd[10] = {0,0,0,0,0,0,0,0,0,0};
		int input_seq[26] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		int blk = 0, val = 0, sum = 0;


		for (int i = 0; i < noutput_items; i++)
		{

			// Isolate the input sequence
			for (int x = 0; x < 26; x++)
			{
				input_seq[x] = in[i+x];  // Data from 0 to 15, check from 16 to 25
			}

			if (!d_syncd)
			{
				//
				// Perform the synchronization as described in
				// Appendices B and C of the U.S. RBDS Standard - April 1998
				//

				// Calculate the syndrome for the current input sequence
				syndrome_calc(&input_seq[0], &synd[0]);
				
				// Check against known syndromes
				blk = sync_impl::block_ident(&synd[0]);

				if ((blk > -1) && (blk < 5))
				{
					// We're in presync!

					// Check if we're sync'ed
					if ((d_sync_cntr == 26) && (d_last_syndrome != -1))
					{
						// Were we in presync before?
						if ((blk == (d_last_syndrome + 1)) || ((blk == 4) && (d_last_syndrome == 2 )) || ((blk == 3) && (d_last_syndrome == 1 )) || ((blk == 0) && (d_last_syndrome == 4 )))
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
						for (int k = 0; k < 16; k++)
						{
							printf("%d ", input_seq[k]);
							if (k % 4 == 3) printf(" ");
						}	
						printf("] %d <- %d p\n", blk, d_last_syndrome);
					}

					d_last_syndrome = blk;
					d_sync_cntr = 0;
				}

				// lost presync
				if (d_sync_cntr > 26)
				{
					d_last_syndrome = -1;
					d_syncd = 0;
				}

			}
			else
			{
				// Check if we're sync'ed
				if (d_sync_cntr == 26)
				{

					//
					// Perform data decoding as described in
					// Appendices B (section B.2.2) of the U.S. RBDS Standard - April 1998
					//

					if ((d_last_syndrome == 0) || (d_last_syndrome == 3))
					{
						blk = d_last_syndrome + 1;

						remove_offset_word(blk, &input_seq[0]);

						// Now we check the block for error (or try to fix up to 5 consecutive errors)
						for (int y = 0; y < 7; y++)
						{
							// Calculate the syndrome of the block without the offset word to look for channel errors
							if (syndrome_calc(&input_seq[0], &synd[0]) == 0)
							{
								printf(" [ ");
								for (int k = 0; k < 16; k++)
								{
									printf("%d ", input_seq[k]);
									if (k % 4 == 3) printf(" ");
								}	
								printf("] %d <- %d * (%d)\n", blk, d_last_syndrome, y);

								d_last_syndrome = blk;
								d_sync_cntr = 0;
								break;
							}

							error_correction(&synd[0], &input_seq[0]);
						}

						if (d_last_syndrome != blk)
						{
							printf("Sync lost after block %d\n", d_last_syndrome);

							// We loose sync... :(
							d_last_syndrome = -1;
							d_sync_cntr = 0;
							d_syncd = 0;
						}

					}
					else if (d_last_syndrome == 4)
					{
						blk = 0;

						remove_offset_word(blk, &input_seq[0]);

						// Now we check the block for error (or try to fix up to 5 consecutive errors)
						for (int y = 0; y < 7; y++)
						{
							// Calculate the syndrome of the block without the offset word to look for channel errors
							if (syndrome_calc(&input_seq[0], &synd[0]) == 0)
							{
								printf(" [ ");
								for (int k = 0; k < 16; k++)
								{
									printf("%d ", input_seq[k]);
									if (k % 4 == 3) printf(" ");
								}	
								printf("] %d <- %d * (%d)\n", blk, d_last_syndrome, y);

								d_last_syndrome = blk;
								d_sync_cntr = 0;
								break;
							}

							error_correction(&synd[0], &input_seq[0]);
						}

						if (d_last_syndrome != blk)
						{
							printf("Sync lost after block %d\n", d_last_syndrome);

							// We loose sync... :(
							d_last_syndrome = -1;
							d_sync_cntr = 0;
							d_syncd = 0;
						}
					}
					else if (d_last_syndrome == 2)
					{
						blk = 4;

						remove_offset_word(blk, &input_seq[0]);

						// Now we check the block for error (or try to fix up to 5 consecutive errors)
						for (int y = 0; y < 7; y++)
						{
							// Calculate the syndrome of the block without the offset word to look for channel errors
							if (syndrome_calc(&input_seq[0], &synd[0]) == 0)
							{
								printf(" [ ");
								for (int k = 0; k < 16; k++)
								{
									printf("%d ", input_seq[k]);
									if (k % 4 == 3) printf(" ");
								}	
								printf("] %d <- %d * (%d)\n", blk, d_last_syndrome, y);

								d_last_syndrome = blk;
								d_sync_cntr = 0;
								break;
							}

							error_correction(&synd[0], &input_seq[0]);
						}

						if (d_last_syndrome != blk)
						{
							printf("Sync lost after block %d\n", d_last_syndrome);

							// We loose sync... :(
							d_last_syndrome = -1;
							d_sync_cntr = 0;
							d_syncd = 0;
						}
					}
					else
					{
						blk = 2;

						remove_offset_word(blk, &input_seq[0]);

						// Now we check the block for error (or try to fix up to 5 consecutive errors)
						for (int y = 0; y < 7; y++)
						{
							// Calculate the syndrome of the block without the offset word to look for channel errors
							if (syndrome_calc(&input_seq[0], &synd[0]) == 0)
							{
								printf(" [ ");
								for (int k = 0; k < 16; k++)
								{
									printf("%d ", input_seq[k]);
									if (k % 4 == 3) printf(" ");
								}	
								printf("] %d <- %d * (%d)\n", blk, d_last_syndrome, y);

								d_last_syndrome = blk;
								d_sync_cntr = 0;
								break;
							}

							error_correction(&synd[0], &input_seq[0]);
						}

						if (d_last_syndrome != blk)
						{
							// Ooops, couldnt decode a 2.. lets try a 3:
							blk = 3;

							// Re-isolate the input sequence
							for (int x = 0; x < 26; x++)
							{
								input_seq[x] = in[i+x];  // Data from 0 to 15, check from 16 to 25
							}

							remove_offset_word(blk, &input_seq[0]);

							// Now we check the block for error (or try to fix up to 5 consecutive errors)
							for (int y = 0; y < 7; y++)
							{
								// Calculate the syndrome of the block without the offset word to look for channel errors
								if (syndrome_calc(&input_seq[0], &synd[0]) == 0)
								{
									printf(" [ ");
									for (int k = 0; k < 16; k++)
									{
										printf("%d ", input_seq[k]);
										if (k % 4 == 3) printf(" ");
									}	
									printf("] %d <- %d * (%d)\n", blk, d_last_syndrome, y);

									d_last_syndrome = blk;
									d_sync_cntr = 0;
									break;
								}

								error_correction(&synd[0], &input_seq[0]);
							}

							if (d_last_syndrome != blk)
							{
								printf("Sync lost after block %d\n", d_last_syndrome);

								// We loose sync... :(
								d_last_syndrome = -1;
								d_sync_cntr = 0;
								d_syncd = 0;
							}
						}
					}

				}

			}

			d_sync_cntr++;

			out[i] = in[i];
			syncd[i] = d_last_syndrome;

		}

        consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

    int sync_impl::syndrome_calc(const int* in, int* res)
    {	
    	/*
			Calculates the syndrome of a received block as seen in
			Appendix B of the U.S. RBDS Standard - April 1998

			Input:
				const int* in 		: the data bit vector (size 26)
				int* res 			: where the calculated syndrome vector will be written (size 10)

    	*/

    	// TODO: use the shift register implementation to calculate the syndrome... its much faster
    	int tmp = 0, sum = 0;

    	// Syndrome matrix multiplication
		for (int y = 0; y < 10; y++)
		{
			tmp = 0;

			for (int x = 0; x < 26; x++)
			{
				tmp += d_parity_chk[x][y] * in[x];
			}

			res[y] = tmp % 2;

			sum += res[y];
		}

		return sum;
    }

    int sync_impl::block_ident(const int* synd)
    {
    	/*
			Identifies which block has been received based on the syndrome as seen in
			Appendix B of the U.S. RBDS Standard - April 1998

			Input:
				const int* synd : a previously calculated syndrome vector (size 10)
			Output:
				int 	 		: the block number (-1 if no block was detected)
    	*/

    	// Check against known syndromes
		for (int x = 0; x < 5; x++)
		{
			if (memcmp(synd, &d_syndromes[x * 10], sizeof(int) * 10) == 0)
			{
				return x;
			}
		}

		// If nothing is identified, return -1
		return -1;
    }

    void sync_impl::remove_offset_word(const int blk, int* in)
    {
  //   	printf(" [ ");
		// for (int k = 0; k < 26; k++)
		// {
		// 	printf("%d ", in[k]);
		// }	
		// printf("] before offset removal\n");

    	// Remove the check word
		for (int x = 0; x < 10; x++)
		{
			in[16 + x] = (in[16 + x] + d_checks[(blk * 10) + x]) % 2;
		}
    }

    void sync_impl::error_correction(const int* synd, int* in)
    {
    	/*
			Implementation of the error correction as seen in Appendix B of the U.S. RBDS Standard - April 1998

			Corrects one bit at a time in a sequence of up to 5 erroneous bits. If bits are not in sequence,
			no correction is done. To correct the remainig bits, update the syndrome and call this function again.

			Input:
				const int* synd : a previously calculated syndrome vector (size 10)
				int* in 		: the data bit vector (will be updated with the correct bit) (size 26)
    	*/

    	int synd_reg[10] = {0,0,0,0,0,0,0,0,0,0};
    	int synd_tmp[10] = {0,0,0,0,0,0,0,0,0,0};
    	int error_mask[26] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  //   	printf(" [ ");
		// for (int k = 0; k < 26; k++)
		// {
		// 	printf("%d ", in[k]);
		// }	
		// printf("] !\n");

    	// Initializing the syndrome register for initial data correction:
    	for (int i = 0; i < 10; i++)
    	{
    		synd_reg[9 - i] = synd[i];
    	}

		// printf(" s < ");
		// for (int k = 0; k < 10; k++)
		// {
		// 	printf("%d ", synd_reg[k]);
		// }	
		// printf(">\n");

    	// Syndrome matrix multiplication
		for (int i = 0; i < 26; i++)
		{
			// Check to see if there's a bit to correct
			if (!(synd_reg[0] || synd_reg[1] || synd_reg[2] || synd_reg[3] || synd_reg[4]) && synd_reg[9])
			{
				error_mask[i] = synd_reg[9];
				break;
			}

			if (i < 16)
			{
				// Rotation of the register and xor operations
				synd_tmp[0] = synd_reg[9];  					synd_tmp[1] = synd_reg[0];  synd_tmp[2] = synd_reg[1];                      synd_tmp[3] = (synd_reg[9] + synd_reg[2]) % 2;  synd_tmp[4] = (synd_reg[9] + synd_reg[3]) % 2;
				synd_tmp[5] = (synd_reg[9] + synd_reg[4]) % 2;  synd_tmp[6] = synd_reg[5];  synd_tmp[7] = (synd_reg[9] + synd_reg[6]) % 2;  synd_tmp[8] = (synd_reg[9] + synd_reg[7]) % 2;  synd_tmp[9] = synd_reg[8];
			}
			else
			{
				// Rotation of the register
				synd_tmp[0] = 0;  			synd_tmp[1] = synd_reg[0];  synd_tmp[2] = synd_reg[1];  synd_tmp[3] = synd_reg[2];  synd_tmp[4] = synd_reg[3];
				synd_tmp[5] = synd_reg[4];  synd_tmp[6] = synd_reg[5];  synd_tmp[7] = synd_reg[6];  synd_tmp[8] = synd_reg[7];  synd_tmp[9] = synd_reg[8];
			}

			// Update the register
			for (int k = 0; k < 10; k++)
			{
				synd_reg[k] = synd_tmp[k];
			}

			// printf(" %d < ", i);
			// for (int k = 0; k < 10; k++)
			// {
			// 	printf("%d ", synd_reg[k]);
			// }
			// printf(">\n");

		}

		// Applying the data correction:
    	for (int i = 0; i < 26; i++)
    	{
    		in[i] = error_mask[i] ^ in[i];
    	}
    }

  } /* namespace fmrds */
} /* namespace gr */

