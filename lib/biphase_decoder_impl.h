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

#ifndef INCLUDED_FMRDS_BIPHASE_DECODER_IMPL_H
#define INCLUDED_FMRDS_BIPHASE_DECODER_IMPL_H

#include <fmrds/biphase_decoder.h>

namespace gr {
  namespace fmrds {

    class biphase_decoder_impl : public biphase_decoder
    {
     private:
      float d_out_bit;
      int d_offset;
      int d_offset_cntr;
      int d_syncd;
      long long int d_resync_cntr;

     public:
      biphase_decoder_impl();
      ~biphase_decoder_impl();

      // Where all the action really happens
      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);

      // sign function
      int sgn(float val);
    };

  } // namespace fmrds
} // namespace gr

#endif /* INCLUDED_FMRDS_BIPHASE_DECODER_IMPL_H */

