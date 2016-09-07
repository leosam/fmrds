#define FMRDS_API

%include <gnuradio.i>
%include "fmrds_swig_doc.i"

%{
#include "fmrds/div16_ff.h"
#include "fmrds/biphase_decoder.h"
#include "fmrds/diff_decoder.h"
#include "fmrds/sync.h"
#include "fmrds/data_decoder.h"
#include "fmrds/time_sync.h"
%}


%include "fmrds/div16_ff.h"
GR_SWIG_BLOCK_MAGIC2(fmrds, div16_ff);
%include "fmrds/biphase_decoder.h"
GR_SWIG_BLOCK_MAGIC2(fmrds, biphase_decoder);
%include "fmrds/diff_decoder.h"
GR_SWIG_BLOCK_MAGIC2(fmrds, diff_decoder);
%include "fmrds/sync.h"
GR_SWIG_BLOCK_MAGIC2(fmrds, sync);
%include "fmrds/data_decoder.h"
GR_SWIG_BLOCK_MAGIC2(fmrds, data_decoder);
%include "fmrds/time_sync.h"
GR_SWIG_BLOCK_MAGIC2(fmrds, time_sync);
