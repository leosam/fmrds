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


#ifndef INCLUDED_FMRDS_TIME_SYNC_H
#define INCLUDED_FMRDS_TIME_SYNC_H

#include <fmrds/api.h>
#include <gnuradio/sync_block.h>
#include <stdio.h>

namespace gr {
  namespace fmrds {

    /*!
     * \brief <+description of block+>
     * \ingroup fmrds
     *
     */
    class FMRDS_API time_sync : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<time_sync> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fmrds::time_sync.
       *
       * To avoid accidental use of raw pointers, fmrds::time_sync's
       * constructor is in a private implementation
       * class. fmrds::time_sync::make is the public interface for
       * creating new instances.
       */
      static sptr make(const int n_samples, const int n_resync);
    };

  } // namespace fmrds
} // namespace gr

#endif /* INCLUDED_FMRDS_TIME_SYNC_H */

