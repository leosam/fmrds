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


#ifndef INCLUDED_FMRDS_BIPHASE_DECODER_H
#define INCLUDED_FMRDS_BIPHASE_DECODER_H

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
    class FMRDS_API biphase_decoder : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<biphase_decoder> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fmrds::biphase_decoder.
       *
       * To avoid accidental use of raw pointers, fmrds::biphase_decoder's
       * constructor is in a private implementation
       * class. fmrds::biphase_decoder::make is the public interface for
       * creating new instances.
       */
      static sptr make();
    };

  } // namespace fmrds
} // namespace gr

#endif /* INCLUDED_FMRDS_BIPHASE_DECODER_H */

