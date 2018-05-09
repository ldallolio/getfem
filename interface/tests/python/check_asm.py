#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Python GetFEM++ interface
#
# Copyright (C) 2018-2018 Yves Renard.
#
# This file is a part of GetFEM++
#
# GetFEM++  is  free software;  you  can  redistribute  it  and/or modify it
# under  the  terms  of the  GNU  Lesser General Public License as published
# by  the  Free Software Foundation;  either version 2.1 of the License,  or
# (at your option) any later version.
# This program  is  distributed  in  the  hope  that it will be useful,  but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or  FITNESS  FOR  A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
# You  should  have received a copy of the GNU Lesser General Public License
# along  with  this program;  if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.
#
############################################################################
"""  test high generic assembly language.

  This program is used to check that Python-GetFEM interface, and more
  generally GetFEM are working. It focuses on testing some operations
  of the high generic assembly language.

  $Id$
"""
import numpy as np
import getfem as gf
import os


NX = 4
m = gf.Mesh('triangles grid', np.arange(0,1+1./NX,1./NX),
            np.arange(0,1+1./NX,1./NX))     # Structured mesh
fem = gf.Fem('FEM_PK(2,1)')
mfu = gf.MeshFem(m, 1); mfu.set_fem(fem)    # Lagrange P1 scalar fem
mfv = gf.MeshFem(m, 3); mfv.set_fem(fem)    # Lagrange P1 vector fem
mfw = gf.MeshFem(m, 2); mfw.set_fem(fem)    # Lagrange P1 vector fem
mim = gf.MeshIm(m, gf.Integ('IM_TRIANGLE(4)'))


U = mfu.eval('x+y')
V = mfv.eval('[x*y, x*y, x*y]')
W = mfw.eval('[x*y, x*y]')


md = gf.Model('real')

md.add_fem_variable('u', mfu)
md.set_variable('u', U)
md.add_fem_variable('v', mfv)
md.set_variable('v', V)
md.add_fem_variable('w', mfw)
md.set_variable('w', W)


# Simple test on the integral of u
result = gf.asm('generic', mim, 0, "u", -1, md)
if (abs(result-1) > 1e-8) : print "Bad value"; exit(1)

# Single contraction and comparison with Trace
result1 = gf.asm('generic', mim, 0,
                 "Def P(a):=a*(a'); Contract(P(Grad_v), 1, 2)", -1, md)
result2 = gf.asm('generic', mim, 0,
                 "Def P(a):=a*(a'); Trace(P(Grad_v))", -1, md)
if (abs(result1-result2) > 1e-8) : print "Bad value"; exit(1)

# Constant order 3 tensor contraction test
result1 = gf.asm('generic', mim, 0,
                 "Contract([[[1,1],[2,2]],[[1,1],[2,2]]], 1, 2)", -1, md)
result2 = np.array([3., 3.]);
if (np.linalg.norm(result1-result2) > 1e-8) : print "Bad value"; exit(1)

# Single contraction, comparison with "*"
result1 = gf.asm('generic', mim, 0, "Contract(Grad_v, 2, Grad_u, 1)", -1, md)
result2 = gf.asm('generic', mim, 0, "Grad_v * Grad_u", -1, md)
if (np.linalg.norm(result1-result2) > 1e-8) : print "Bad value"; exit(1)

# Double contraction order one expression, comparison with ":"
result1 = gf.asm('generic', mim, 1,
                 "Contract(Grad_v, 1, 2, Grad_Test_v, 1, 2)", -1, md)
result2 = gf.asm('generic', mim, 1, "Grad_v : Grad_Test_v", -1, md)
if (np.linalg.norm(result1-result2) > 1e-8) : print "Bad value"; exit(1)

# Double contraction order two expression, comparison with ":"
result1 = gf.asm('generic', mim, 2,
                 "Contract(Grad_Test2_v, 1, 2, Grad_Test_v, 1, 2)", -1, md)
result2 = gf.asm('generic', mim, 2, "Grad_Test2_v : Grad_Test_v", -1, md)
if (np.linalg.norm(result1.full()-result2.full()) > 1e-8) :
  print "Bad value"; exit(1)
result1 = gf.asm('generic', mim, 2,
                 "Contract(Grad_Test_v, 2, 1, Grad_Test2_v, 2, 1)", -1, md)
if (np.linalg.norm(result1.full()-result2.full()) > 1e-8) :
  print "Bad value"; exit(1)

print 'Assembly string "Def P(u):= Grad(u); P(Grad(u)+[1;1])" gives:'
res = gf.asm('expression analysis', 'Def P(u):= Grad(u); P(Grad(u)+[1;1])',  mim, 0, md)
if (res != "(Hess_u)"): print "Bad gradient"; exit(1)

print 'Assembly string "Grad(Grad_u\')" gives:'
res = gf.asm('expression analysis', "Grad(Grad_u')",  mim, 0, md)
if (res != "(Reshape(Hess_u, 1, 2, 2))"): print "Bad gradient"; exit(1)

print 'Assembly string "Grad(Sym(Grad_w))" gives:'
res = gf.asm('expression analysis', "Grad(Sym(Grad_w))",  mim, 0, md)
if (res != "((Hess_w+(Hess_w'))*0.5)"): print "Bad gradient"; exit(1)

print 'Assembly string "Grad(Skew(Grad_w))" gives:'
res = gf.asm('expression analysis', "Grad(Skew(Grad_w))",  mim, 0, md)
if (res != "((Hess_w-(Hess_w'))*0.5)"): print "Bad gradient"; exit(1)

print 'Assembly string "Grad(Grad_w*Grad_u)" gives:'
res = gf.asm('expression analysis', "Grad(Grad_w*Grad_u)",  mim, 0, md)
print res
if (res != "(Contract(Hess_w, 2, Grad_u, 1)+(Grad_w.Hess_u))"):
  print "Bad gradient"; exit(1)

print 'Assembly string "Grad(u*Grad_w)" gives:'
res = gf.asm('expression analysis', "Grad(u*Grad_w)",  mim, 0, md)
if (res != "((Grad_w@Grad_u)+(u*Hess_w))"): print "Bad gradient"; exit(1)

print 'Assembly string "Grad(Grad_w:Id(meshdim))" gives:'
res = gf.asm('expression analysis', "Grad(Grad_w:Id(meshdim))",  mim, 0, md)
if (res != "(Contract([[1,0],[0,1]], 1, 2, Hess_w, 1, 2))"):
  print "Bad gradient"; exit(1)

print 'Assembly string "Grad(Grad_w:Id(meshdim))" gives:'
res = gf.asm('expression analysis', "Grad(Grad_w@Grad_v)",  mim, 0, md)
if (res != "(Index_move_last((Hess_w@Grad_v), 3)+(Grad_w@Hess_v))"):
  print "Bad gradient"; exit(1)
  
print 'Assembly string "Grad(Grad_w.Grad_w)" gives:'
res = gf.asm('expression analysis', "Grad(Grad_w.Grad_w)",  mim, 0, md)
if (res !=
    "(Index_move_last(Contract(Hess_w, 2, Grad_w, 1), 2)+(Grad_w.Hess_w))"):
  print "Bad gradient"; exit(1)

print 'Assembly string "Grad(Grad_w./Grad_w)" gives:'
res = gf.asm('expression analysis', "Grad(Grad_w./Grad_w)",  mim, 0, md)
if (res !=
    "((Hess_w./(Grad_w@[1,1]))-(((Grad_w./sqr(Grad_w))@[1,1]).*Hess_w))"):
  print "Bad gradient"; exit(1)
  
print 'Assembly string "Grad(Grad_w/u)" gives:'
res = gf.asm('expression analysis', "Grad(Grad_w/u)",  mim, 0, md)
if (res != "((Hess_w/u)-((Grad_w/sqr(u))@Grad_u))"):
  print "Bad gradient"; exit(1)
  
print 'Assembly string "Grad([u,u; 2,1; u,u])" gives:'
res = gf.asm('expression analysis', "Grad([u,u; 2,1; u,u])",  mim, 0, md)
if (res != "([[[Grad_u(1),0,Grad_u(1)],[Grad_u(1),0,Grad_u(1)]],[[Grad_u(2),0,Grad_u(2)],[Grad_u(2),0,Grad_u(2)]]])"):
  print "Bad gradient"; exit(1)

print 'Assembly string "Grad([[u,2,u],[u,1,u]])" gives:'
res = gf.asm('expression analysis', "Grad([[u,2,u],[u,1,u]])",  mim, 0, md)
if (res != "([[[Grad_u(1),0,Grad_u(1)],[Grad_u(1),0,Grad_u(1)]],[[Grad_u(2),0,Grad_u(2)],[Grad_u(2),0,Grad_u(2)]]])"):
  print "Bad gradient"; exit(1)
  
print 'Assembly string "Grad([u,u])" gives:'
res = gf.asm('expression analysis', "Grad([u,u])",  mim, 0, md)

print 'Assembly string "Grad([u;u])" gives:'
res = gf.asm('expression analysis', "Grad([u,u])",  mim, 0, md)



print 'Assembly string "Grad(Reshape(Grad_w, 1, 4))" gives:'
res = gf.asm('expression analysis', "Grad(Reshape(Grad_w, 1, 4))",  mim, 0, md)
if (res != "(Reshape(Hess_w, 1, 4, 2))"): print "Bad gradient"; exit(1)

print 'Assembly string "Grad(Grad_w(1,2))" gives:'
res = gf.asm('expression analysis', "Grad(Grad_w(1,2))",  mim, 0, md)
if (res != "(Hess_w(1, 2, :))"): print "Bad gradient"; exit(1)

print 'Assembly string "Grad(Index_move_last(Grad_w, 1))" gives:'
res = gf.asm('expression analysis', "Grad(Index_move_last(Grad_w, 1))", mim, 0, md)
if (res != "(Swap_indices(Index_move_last(Hess_w, 1), 2, 3))"):
  print "Bad gradient"; exit(1)

print 'Assembly string "Grad(Contract(Grad_w, 1, 2, Grad_w, 1, 2))" gives:'
res = gf.asm('expression analysis', "Grad(Contract(Grad_w, 1, 2, Grad_w, 1, 2))", mim, 0, md)
if (res != "(Contract(Hess_w, 1, 2, Grad_w, 1, 2)+Contract(Grad_w, 1, 2, Hess_w, 1, 2))"):
  print "Bad gradient"; exit(1)


str = "[1;2;3]"; print 'Assembly string "%s" gives:' % str
res = gf.asm('expression analysis', str,  mim, 0, md)

str = "[1,2,3]"; print 'Assembly string "%s" gives:' % str
res = gf.asm('expression analysis', str,  mim, 0, md)


str = "[u;u;u].[u;u;u]"; print 'Assembly string "%s" gives:' % str
res = gf.asm('expression analysis', str,  mim, 2, md)



