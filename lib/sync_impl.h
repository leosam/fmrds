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

#ifndef INCLUDED_FMRDS_SYNC_IMPL_H
#define INCLUDED_FMRDS_SYNC_IMPL_H

#include <fmrds/sync.h>

namespace gr {
  namespace fmrds {

    class sync_impl : public sync
    {

     public:
      sync_impl();
      ~sync_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);

      int syndrome_calc(const int* in, int* res);

      int error_correction(const int* synd, int* in);

      void remove_offset_word(const int blk, int* in);

      int block_ident(const int* synd);

      int decode(const int blk, const int* in, int* synd, int* out);

      private:
        // Parity check matrix
        int d_parity_chk[26][10];

        // Precalculated syndromes for the check words
        int d_syndromes[50];

        // Check words
        int d_checks[50];

        // Last identified syndrome
        //      0->A, 1->B, 2->C, 3->C', 4->D
        //     -1->"no sync"              
        int d_last_syndrome;

        // Last identified syndrome
        //      occurence
        int d_last_seen;

        // Sync counter
        int d_sync_cntr;

        // Overall counter
        int d_overall;

        // Sync'ed flag
        int d_syncd;
    };

  } // namespace fmrds
} // namespace gr

#endif /* INCLUDED_FMRDS_SYNC_IMPL_H */

