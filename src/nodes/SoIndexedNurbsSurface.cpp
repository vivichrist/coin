/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2000 by Systems in Motion. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  version 2.1 as published by the Free Software Foundation. See the
 *  file LICENSE.LGPL at the root directory of the distribution for
 *  more details.
 *
 *  If you want to use Coin for applications not compatible with the
 *  LGPL, please contact SIM to acquire a Professional Edition license.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  http://www.sim.no support@sim.no Voice: +47 22114160 Fax: +47 22207097
 *
\**************************************************************************/

/*!
  \class SoIndexedNurbsSurface SoIndexedNurbsSurface.h Inventor/nodes/SoIndexedNurbsSurface.h
  \brief The SoIndexedNurbsSurface class can be used to render NURBS surfaces.
  \ingroup nodes

  It is very similar to the SoNurbsSurface class, but control points
  can be specified using indices.
*/

#include <Inventor/nodes/SoIndexedNurbsSurface.h>
#include <Inventor/nodes/SoSubNodeP.h>
#include <Inventor/elements/SoCoordinateElement.h>
#include <Inventor/elements/SoDrawStyleElement.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/elements/SoPickStyleElement.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/misc/SoGL.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/errors/SoDebugError.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>
#include <src/misc/GLUWrapper.h>

#include <coindefs.h> // COIN_STUB()

/*!
  \var SoSFInt32 SoIndexedNurbsSurface::numUControlPoints
  Number of control points in the U direction.
*/
/*!
  \var SoSFInt32 SoIndexedNurbsSurface::numVControlPoints
  Number of control points in the V direction.
*/
/*!
  \var SoSFInt32 SoIndexedNurbsSurface::numSControlPoints
  Number of control points in the S direction.
*/
/*!
  \var SoSFInt32 SoIndexedNurbsSurface::numTControlPoints
  Number of control points in the T direction.
*/
/*!
  \var SoMFFloat SoIndexedNurbsSurface::uKnotVector
  The Bezier knot vector for the U direction.
*/
/*!
  \var SoMFFloat SoIndexedNurbsSurface::vKnotVector
  The Bezier knot vector for the V direction.
*/
/*!
  \var SoMFFloat SoIndexedNurbsSurface::sKnotVector
  The Bezier knot vector for the S direction.
*/
/*!
  \var SoMFFloat SoIndexedNurbsSurface::tKnotVector
  The Bezier knot vector for the T direction.
*/

/*!
  \var SoMFInt32 SoIndexedNurbsSurface::coordIndex
  The coordinate control point indices.
*/
/*!
  \var SoMFInt32 SoIndexedNurbsSurface::textureCoordIndex
  The texture coordinate control point indices.
*/

// *************************************************************************

SO_NODE_SOURCE(SoIndexedNurbsSurface);

/*!
  Constructor.
*/
SoIndexedNurbsSurface::SoIndexedNurbsSurface()
{
  SO_NODE_INTERNAL_CONSTRUCTOR(SoIndexedNurbsSurface);

  SO_NODE_ADD_FIELD(numUControlPoints, (0));
  SO_NODE_ADD_FIELD(numVControlPoints, (0));
  SO_NODE_ADD_FIELD(coordIndex, (0));
  SO_NODE_ADD_FIELD(uKnotVector, (0));
  SO_NODE_ADD_FIELD(vKnotVector, (0));
  SO_NODE_ADD_FIELD(numSControlPoints, (0));
  SO_NODE_ADD_FIELD(numTControlPoints, (0));
  SO_NODE_ADD_FIELD(textureCoordIndex, (-1));
  SO_NODE_ADD_FIELD(sKnotVector, (0));
  SO_NODE_ADD_FIELD(tKnotVector, (0));

  this->nurbsrenderer = NULL;
}

/*!
  Destructor.
*/
SoIndexedNurbsSurface::~SoIndexedNurbsSurface()
{
  if (this->nurbsrenderer) {
    GLUWrapper()->gluDeleteNurbsRenderer(this->nurbsrenderer);
  }
}

// doc from parent
void
SoIndexedNurbsSurface::initClass(void)
{
  SO_NODE_INTERNAL_INIT_CLASS(SoIndexedNurbsSurface);
}

/*!
  Calculates the bounding box of all control points and sets the center
  to the average of these points.
*/
void
SoIndexedNurbsSurface::computeBBox(SoAction * action,
                                   SbBox3f & box, SbVec3f & center)
{
  SoState * state = action->getState();
  const SoCoordinateElement * coordelem =
    SoCoordinateElement::getInstance(state);

  int num = this->coordIndex.getNum();

  box.makeEmpty();
  SbVec3f acccenter(0.0f, 0.0f, 0.0f);
  SbVec3f tmp3D;

  if (coordelem->is3D()) {
    const SbVec3f * coords = coordelem->getArrayPtr3();
    assert(coords);
    for (int i = 0; i < num; i++) {
      tmp3D = coords[this->coordIndex[i]];
      box.extendBy(tmp3D);
      acccenter += tmp3D;
    }
  }
  else {
    const SbVec4f * coords = coordelem->getArrayPtr4();
    assert(coords);
    for (int i = 0; i< num; i++) {
      SbVec4f tmp = coords[this->coordIndex[i]];
      float mul = (tmp[3] != 0.0f) ? 1.0f / tmp[4] : 1.0f;
      tmp3D.setValue(tmp[0]*mul, tmp[1]*mul, tmp[2]*mul);
      box.extendBy(tmp3D);
      acccenter += tmp3D;
    }
  }
  if (num) center = acccenter / float(num);
}

/*!
  FIXME: write doc
*/
void
SoIndexedNurbsSurface::GLRender(SoGLRenderAction * action)
{
  if (!this->shouldGLRender(action)) return;

  // initialize current material
  SoMaterialBundle mb(action);
  mb.sendFirst();

  glEnable(GL_AUTO_NORMAL);
  this->doNurbs(action, TRUE);
  glDisable(GL_AUTO_NORMAL);
}

// doc from parent
void
SoIndexedNurbsSurface::rayPick(SoRayPickAction * action)
{
  if (GLUWrapper()->versionMatchesAtLeast(1, 3, 0)) {
    SoShape::rayPick(action); // do normal generatePrimitives() pick
  }
  else {
    if (!this->shouldRayPick(action)) return;
    static SbBool firstpick = TRUE;
    if (firstpick) {
      firstpick = FALSE;
      SoDebugError::postWarning("SoIndexedNurbsSurface::rayPick",
                                "Proper NURBS picking requires\n"
                                "GLU version 1.3. Picking is done on bounding box.");
    }
    SoState * state = action->getState();
    state->push();
    SoPickStyleElement::set(state, this, SoPickStyleElement::BOUNDING_BOX);
    (void)this->shouldRayPick(action); // this will cause a pick on bbox
    state->pop();
  }
}

//
// used only for GLU callbacks
//
typedef struct {
  SoAction * action;
  SoIndexedNurbsSurface * thisp;
  SoPrimitiveVertex vertex;
} coin_ins_cbdata;


// doc from parent
void
SoIndexedNurbsSurface::getPrimitiveCount(SoGetPrimitiveCountAction * /* action */)
{
  COIN_STUB();
}

/*!
  Not implemented. Should probably have been private in OIV.
*/
void
SoIndexedNurbsSurface::sendPrimitive(SoAction * ,  SoPrimitiveVertex *)
{
  COIN_STUB();
}

// doc from parent
void
SoIndexedNurbsSurface::generatePrimitives(SoAction * action)
{
  this->doNurbs(action, FALSE);
}

/*!
  Overloaded to return NULL.
*/
SoDetail *
SoIndexedNurbsSurface::createTriangleDetail(SoRayPickAction * /* action */,
                                            const SoPrimitiveVertex * /*v1*/,
                                            const SoPrimitiveVertex * /*v2*/,
                                            const SoPrimitiveVertex * /*v3*/,
                                            SoPickedPoint * /* pp */)
{
  return NULL;
}


void
SoIndexedNurbsSurface::doNurbs(SoAction * action, const SbBool glrender)
{
  if (GLUWrapper()->available == 0 || !GLUWrapper()->gluNewNurbsRenderer) {
#if COIN_DEBUG
    static int first = 1;
    if (first) {
      SoDebugError::postInfo("SoIndexedNurbsCurve::doNurbs",
                             "Looks like your GLU library doesn't have NURBS "
                             "functionality");
      first = 0;
    }
#endif // COIN_DEBUG
    return;
  }

  if (!this->coordIndex.getNum()) return;

  if (this->nurbsrenderer == NULL) {
    this->nurbsrenderer = GLUWrapper()->gluNewNurbsRenderer();

    if (GLUWrapper()->versionMatchesAtLeast(1, 3, 0)) {
      GLUWrapper()->gluNurbsCallback(this->nurbsrenderer, (GLenum) GLU_W_NURBS_BEGIN_DATA, (gluNurbsCallback_cb_t)tessBegin);
      GLUWrapper()->gluNurbsCallback(this->nurbsrenderer, (GLenum) GLU_W_NURBS_TEXTURE_COORD_DATA, (gluNurbsCallback_cb_t)tessTexCoord);
      GLUWrapper()->gluNurbsCallback(this->nurbsrenderer, (GLenum) GLU_W_NURBS_NORMAL_DATA, (gluNurbsCallback_cb_t)tessNormal);
      GLUWrapper()->gluNurbsCallback(this->nurbsrenderer, (GLenum) GLU_W_NURBS_VERTEX_DATA, (gluNurbsCallback_cb_t)tessVertex);
      GLUWrapper()->gluNurbsCallback(this->nurbsrenderer, (GLenum) GLU_W_NURBS_END_DATA, (gluNurbsCallback_cb_t)tessEnd);
    }
  }

  if (GLUWrapper()->versionMatchesAtLeast(1, 3, 0)) {
    coin_ins_cbdata cbdata;
    if (!glrender) {
      GLUWrapper()->gluNurbsCallbackData(this->nurbsrenderer, &cbdata);
      cbdata.action = action;
      cbdata.thisp = this;
      cbdata.vertex.setNormal(SbVec3f(0.0f, 0.0f, 1.0f));
      cbdata.vertex.setMaterialIndex(0);
      cbdata.vertex.setTextureCoords(SbVec4f(0.0f, 0.0f, 0.0f, 1.0f));
      cbdata.vertex.setPoint(SbVec3f(0.0f, 0.0f, 0.0f));
      cbdata.vertex.setDetail(NULL);
    }
  }

  SbBool texindex =
    this->textureCoordIndex.getNum() &&
    this->textureCoordIndex[0] >= 0;

  int displaymode = (int) GLU_W_FILL;
  if (glrender) {
    switch (SoDrawStyleElement::get(action->getState())) {
    case SoDrawStyleElement::LINES:
      displaymode = (int) GLU_W_OUTLINE_POLYGON;
      break;
    case SoDrawStyleElement::POINTS:
      // not possible to draw NURBS as points using GLU...
      displaymode = (int) GLU_W_OUTLINE_PATCH;
      break;
    default:
      break;
    }
  }
  GLUWrapper()->gluNurbsProperty(this->nurbsrenderer, (GLenum) GLU_W_DISPLAY_MODE, displaymode);

  // FIXME: we call sogl_render_nurbs_surface() even on GLU <
  // v1.3. Won't this cause an assert() failure in
  // sogl_render_nurbs_surface()? 20000929 mortene.
  sogl_render_nurbs_surface(action, this, this->nurbsrenderer,
                            this->numUControlPoints.getValue(),
                            this->numVControlPoints.getValue(),
                            this->uKnotVector.getValues(0),
                            this->vKnotVector.getValues(0),
                            this->uKnotVector.getNum(),
                            this->vKnotVector.getNum(),
                            this->numSControlPoints.getValue(),
                            this->numTControlPoints.getValue(),
                            this->sKnotVector.getValues(0),
                            this->tKnotVector.getValues(0),
                            this->sKnotVector.getNum(),
                            this->tKnotVector.getNum(),
                            glrender,
                            this->coordIndex.getNum(),
                            this->coordIndex.getValues(0),
                            texindex ? this->textureCoordIndex.getNum() : 0,
                            texindex ? this->textureCoordIndex.getValues(0) : NULL);
}

void
SoIndexedNurbsSurface::tessBegin(int type, void * data)
{
  coin_ins_cbdata * cbdata = (coin_ins_cbdata*) data;
  TriangleShape shapetype;
  switch ((int)type) {
  case GL_TRIANGLES:
    shapetype = SoShape::TRIANGLES;
    break;
  case GL_TRIANGLE_STRIP:
    shapetype = SoShape::TRIANGLE_STRIP;
    break;
  case GL_TRIANGLE_FAN:
    shapetype = SoShape::TRIANGLE_FAN;
    break;
  case GL_QUADS:
    shapetype = SoShape::QUADS;
    break;
  case GL_QUAD_STRIP:
    shapetype = SoShape::QUAD_STRIP;
    break;
  case GL_POLYGON:
    shapetype = SoShape::POLYGON;
    break;
  default:
    shapetype = SoShape::POINTS; // illegal value
#if COIN_DEBUG && 1 // debug
    SoDebugError::postInfo("SoIndexedNurbsSurface::tessBegin",
                           "unsupported GL enum: 0x%x", type);
#endif // debug
    break;
  }
  if (shapetype != SoShape::POINTS) {
    cbdata->thisp->beginShape(cbdata->action, shapetype, NULL);
  }
}

void
SoIndexedNurbsSurface::tessTexCoord(float * texcoord, void * data)
{
  coin_ins_cbdata * cbdata = (coin_ins_cbdata*) data;
  cbdata->vertex.setTextureCoords(SbVec2f(texcoord[0], texcoord[1]));
}

void
SoIndexedNurbsSurface::tessNormal(float * normal, void * data)
{
  coin_ins_cbdata * cbdata = (coin_ins_cbdata*) data;
  cbdata->vertex.setNormal(SbVec3f(normal[0], normal[1], normal[2]));
}

void
SoIndexedNurbsSurface::tessVertex(float * vertex, void * data)
{
  coin_ins_cbdata * cbdata = (coin_ins_cbdata*) data;
  cbdata->vertex.setPoint(SbVec3f(vertex[0], vertex[1], vertex[2]));
  cbdata->thisp->shapeVertex(&cbdata->vertex);
}

void
SoIndexedNurbsSurface::tessEnd(void * data)
{
  coin_ins_cbdata * cbdata = (coin_ins_cbdata*) data;
  cbdata->thisp->endShape();
}
