// ----------------------------------
// Copyright (c) 2011, Brown University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// (1) Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// (3) Neither the name of Brown University nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY BROWN UNIVERSITY “AS IS” WITH NO
// WARRANTIES OR REPRESENTATIONS OF ANY KIND WHATSOEVER EITHER EXPRESS OR
// IMPLIED, INCLUDING WITHOUT LIMITATION ANY WARRANTY OF DESIGN OR
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, EACH OF WHICH ARE
// SPECIFICALLY DISCLAIMED, NOR ANY WARRANTY OR REPRESENTATIONS THAT THE
// SOFTWARE IS ERROR FREE OR THAT THE SOFTWARE WILL NOT INFRINGE ANY
// PATENT, COPYRIGHT, TRADEMARK, OR OTHER THIRD PARTY PROPRIETARY RIGHTS.
// IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY OR CAUSE OF ACTION, WHETHER IN CONTRACT,
// STRICT LIABILITY, TORT, NEGLIGENCE OR OTHERWISE, ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE. ANY RECIPIENT OR USER OF THIS SOFTWARE ACKNOWLEDGES THE
// FOREGOING, AND ACCEPTS ALL RISKS AND LIABILITIES THAT MAY ARISE FROM
// THEIR USE OF THE SOFTWARE.
// ---------------------------------

/// \file ContrastFilter.cpp
/// \author Andy Loomis

#include <sstream>
#include "ContrastFilter.hpp"

using namespace std;

namespace xromm { namespace gpu {

#define KERNEL_X 16
#define KERNEL_Y 16
#define KERNEL_CODE ContrastFilter_cl
#define KERNEL_NAME "contrast_filter_kernel"

#include "gpu/opencl/kernel/ContrastFilter.cl.h"


static int num_contrast_filters = 0;
static Program contrast_program_;

ContrastFilter::ContrastFilter()
    : Filter(XROMM_GPU_CONTRAST_FILTER,""),
      alpha_(1.0f),
      beta_(1.0f),
      size_(3)
{
    stringstream name_stream;
    name_stream << "ContrastFilter" << (++num_contrast_filters);
    name_ = name_stream.str();
}

void
ContrastFilter::apply(
		const Buffer* input,
		Buffer* output,
		int width,
		int height)
{
	Kernel* kernel = contrast_program_.compile(ContrastFilter_cl, KERNEL_NAME);

    kernel->block2d(KERNEL_X, KERNEL_Y);
    kernel->grid2d((width-1)/KERNEL_X+1, (height-1)/KERNEL_Y+1);

    kernel->addBufferArg(input);
    kernel->addBufferArg(output);
    kernel->addArg(width);
    kernel->addArg(height);
    kernel->addArg(alpha_);
    kernel->addArg(beta_);
    kernel->addArg(size_);

	kernel->launch();

	delete kernel;
}

} } // namespace xromm::opencl
