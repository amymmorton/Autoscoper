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

/// \file GLWidget.cpp
/// \author Benjamin Knorlein, Andy Loomis

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <GL/glew.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#ifdef _WIN32
  #include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "ui/GLWidget.h"

#include <QMouseEvent>

#ifndef _PI
#define _PI 3.141592653
#endif

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setAutoFillBackground(false);

  viewdata.zoom = 1.0f;
    viewdata.zoom_x = 0.0f;
    viewdata.zoom_y = 0.0f;
}

void GLWidget::initializeGL(){
  glDisable(GL_LIGHTING);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.5,0.5,0.5,1.0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void GLWidget::resizeGL(int w, int h){
  viewdata.window_width = w;
    viewdata.window_height = h;

    // Prevent divie by 0.
    if (viewdata.window_height == 0) {
        viewdata.window_height = 1;
    }

    viewdata.ratio = (float)viewdata.window_width/(float)viewdata.window_height;

#if defined(Autoscoper_RENDERING_USE_CUDA_BACKEND)
  // Unregister and delete the pixel buffer if it already exists.
    if (!glIsBufferARB(viewdata.pbo)) {
        CALL_GL(glDeleteBuffersARB(1, &viewdata.pbo));
        CALL_GL(glGenBuffersARB(1, &viewdata.pbo));
    }

  // Create a pixel buffer object.
    CALL_GL(glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, viewdata.pbo));
    CALL_GL(glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB,
                3*viewdata.window_width*viewdata.window_height*sizeof(float),
                0,
                GL_STREAM_DRAW_ARB));
    CALL_GL(glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0));
#elif defined(Autoscoper_RENDERING_USE_OpenCL_BACKEND)
  // Unregister and delete the pixel buffer if it already exists.
  if (!glIsBuffer(viewdata.pbo)) {
        CALL_GL(glDeleteBuffers(1, &viewdata.pbo));
    CALL_GL(glGenBuffers(1, &viewdata.pbo));
    }

  // Create a pixel buffer object.
    //glGenBuffersARB(1, &view->pbo);
  CALL_GL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, viewdata.pbo));
    CALL_GL(glBufferData(GL_PIXEL_UNPACK_BUFFER,
                3*viewdata.window_width*viewdata.window_height*sizeof(float),
                0,
                GL_STREAM_DRAW));
    CALL_GL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
#endif

    if (viewdata.viewport_width < viewdata.window_width) {
        viewdata.viewport_width = viewdata.window_width;
    }

    if (viewdata.viewport_height < viewdata.window_height) {
        viewdata.viewport_height = viewdata.window_height;
    }

    // The viewport needs to be recalculated.
    update_viewport(&viewdata);
}

void GLWidget::update_viewport(ViewData* view)
{
   if (view->m_isStaticView) {

        view->zoom = 1.0f;
        view->zoom_x = 0.0f;
        view->zoom_y = 0.0f;

        view->viewport_x = 0;
        view->viewport_y = 0;
        view->viewport_width = view->window_width;
        view->viewport_height = view->window_height;

        return;
    }

    // A zoom of 1 corresponds to a viewport that is the same size as the
    // viewing window. A zoom of 2 corrseponds to a viewport that is twice the
    // size of the window in each dimension. We don't ever want the viewport to
    // be smaller than the window so we clamp the min to 1.
    if (view->zoom < 1.0f) {
        view->zoom = 1.0f;
    }

    view->viewport_width = (int)(view->window_width*view->zoom);
    view->viewport_height = (int)(view->window_height*view->zoom);

    // Clamp the viewport width and height to the maximum viewport dimensions
    // supported by this opengl implementation.
    /* TODO: This effectively limits the maximum amount of zoom. If we do not do
     * this then the manipulator will not be drawn correctly at extreme scales.
     * We need a between solution to this problem.
    if (view->viewport_width > max_viewport_dims[0]) {
        view->viewport_width = max_viewport_dims[0];
        view->zoom = (float)view->viewport_width/(float)view->window_width;
        view->viewport_height = (int)(view->window_height*view->zoom);
    }
    if (view->viewport_height > max_viewport_dims[1]) {
        view->viewport_height = max_viewport_dims[1];
        view->zoom = (float)view->viewport_height/(float)view->window_height;
        view->viewport_width = (int)(view->window_width*view->zoom);
    }
    */

    // The zoom_x and zoom_y parameters should be normalized between -1 and 1.
    // They determine the location of the window in the viewport. They are
    // clamped so that the window never moves outside of the viewport.
    view->viewport_x = -(int)(view->viewport_width/2.0f*
                             (1.0+view->zoom_x-1.0/view->zoom));
    view->viewport_y = -(int)(view->viewport_height/2.0f*
                             (1.0+view->zoom_y-1.0/view->zoom));

    int min_viewport_x = view->window_width-view->viewport_width;
    int max_viewport_x = 0;
    int min_viewport_y = view->window_height-view->viewport_height;
    int max_viewport_y = 0;

    if (view->viewport_x < min_viewport_x) {
        view->viewport_x = min_viewport_x;
        view->zoom_x = 1.0f/(float)view->zoom-
                       2.0f*view->viewport_x/(float)view->viewport_width-1.0f;
    }
    if (view->viewport_x > max_viewport_x) {
        view->viewport_x = max_viewport_x;
        view->zoom_x = 1.0f/(float)view->zoom-
                       2.0f*view->viewport_x/(float)view->viewport_width-1.0f;
    }
    if (view->viewport_y < min_viewport_y) {
        view->viewport_y = min_viewport_y;
        view->zoom_y = 1.0f/(float)view->zoom-
                       2.0f*view->viewport_y/(float)view->viewport_height-1.0f;
    }
    if (view->viewport_y > max_viewport_y) {
        view->viewport_y = max_viewport_y;
        view->zoom_y = 1.0f/(float)view->zoom-
                       2.0f*view->viewport_y/(float)view->viewport_height-1.0f;
    }
}

