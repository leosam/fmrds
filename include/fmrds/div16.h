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


#ifndef INCLUDED_FMRDS_DIV16_H
#define INCLUDED_FMRDS_DIV16_H

#include <fmrds/api.h>
#include <gr_sync_block.h>

namespace gr {
  namespace fmrds {

    /*!
     * \brief <+description of block+>
     * \ingroup fmrds
     *
     */
    class FMRDS_API div16 : virtual public gr_sync_block
    {
     public:
      typedef boost::shared_ptr<div16> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of fmrds::div16.
       *
       * To avoid accidental use of raw pointers, fmrds::div16's
       * constructor is in a private implementation
       * class. fmrds::div16::make is the public interface for
       * creating new instances.
       */
      static sptr make();
    };

  } // namespace fmrds
} // namespace gr

#endif /* INCLUDED_FMRDS_DIV16_H */

