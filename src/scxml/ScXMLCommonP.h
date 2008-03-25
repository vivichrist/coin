#ifndef COIN_SCXMLCOMMONP_H
#define COIN_SCXMLCOMMONP_H

/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2008 by Kongsberg SIM.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using Coin with software that can not be combined with the GNU
 *  GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Kongsberg SIM about acquiring
 *  a Coin Professional Edition License.
 *
 *  See http://www.coin3d.org/ for more information.
 *
 *  Kongsberg SIM, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  http://www.sim.no/  sales@sim.no  coin-support@coin3d.org
 *
\**************************************************************************/

#define SCXML_SINGLE_OBJECT_API_IMPL(classname, objtype, pointer, singular) \
void                                                                    \
classname::SO__CONCAT(set,singular)(const objtype * obj)                \
{                                                                       \
  this->pointer = obj;                                                  \
}                                                                       \
                                                                        \
const objtype *                                                         \
classname::SO__CONCAT(get,singular)(void) const                         \
{                                                                       \
  return this->pointer;                                                 \
}

#define SCXML_LIST_OBJECT_API_IMPL(classname, objtype, objlist, singular, plural) \
                                                                        \
int                                                                     \
classname::SO__CONCAT(getNum,plural)(void) const                        \
{                                                                       \
  return static_cast<int>(this->objlist.size());                        \
}                                                                       \
                                                                        \
const objtype *                                                         \
classname::SO__CONCAT(get,singular)(int idx) const                      \
{                                                                       \
  assert(idx >= 0 && idx < static_cast<int>(this->objlist.size()));     \
  return this->objlist.at(idx);                                         \
}                                                                       \
                                                                        \
void                                                                    \
classname::SO__CONCAT(add,singular)(const objtype * obj)                \
{                                                                       \
  this->objlist.push_back(obj);                                         \
}                                                                       \
                                                                        \
void                                                                    \
classname::SO__CONCAT(remove,singular)(const objtype * obj)             \
{                                                                       \
  std::vector<const objtype *>::iterator it =                           \
    find(this->objlist.begin(), this->objlist.end(), obj);              \
  assert(it != this->objlist.end());                                    \
  this->objlist.erase(it);                                              \
}                                                                       \
                                                                        \
void                                                                    \
classname::SO__CONCAT(clearAll,plural)(void)                            \
{                                                                       \
  this->objlist.clear();                                                \
}

#endif // !COIN_SCXMLCOMMONP_H