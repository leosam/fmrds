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

#ifndef INCLUDED_FMRDS_TIME_SYNC_IMPL_H
#define INCLUDED_FMRDS_TIME_SYNC_IMPL_H

#include <fmrds/time_sync.h>

namespace gr {
  namespace fmrds {

    class time_sync_impl : public time_sync
    {
     private:
      int d_offset;
      int d_syncd;
      int d_half_cycle_cntr;
      int d_sample_cntr;
      int d_resync_cntr;
      int d_resync_timeout;
      float d_data[112];
      int d_pos[100];
      int d_clock_pos;
      int d_n_sampl;

     public:
      time_sync_impl(const int n_samples, const int n_resync);
      ~time_sync_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace fmrds
} // namespace gr

#endif /* INCLUDED_FMRDS_TIME_SYNC_IMPL_H */

