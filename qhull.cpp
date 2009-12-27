/***** BEGIN LICENSE BLOCK *****

BSD License

Copyright (c) 2005-2009, NIF File Format Library and Tools
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the NIF File Format Library and Tools projectmay not be
   used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

***** END LICENCE BLOCK *****/

#include "qhull.h"
#include <algorithm>
#include <functional>
#include <math.h>

extern "C"
{
#include "qhull/src/qhull_a.h"

#include "qhull/src/qhull.c"
#include "qhull/src/mem.c"
#include "qhull/src/qset.c"
#include "qhull/src/geom.c"
#include "qhull/src/merge.c"
#include "qhull/src/poly.c"
#include "qhull/src/io.c"
#include "qhull/src/stat.c"
#include "qhull/src/global.c"
#include "qhull/src/user.c"
#include "qhull/src/poly2.c"
#include "qhull/src/geom2.c"
};

QVector<Triangle> compute_convex_hull( const QVector<Vector3>& verts )
{  
	QVector<Triangle> tris;
	
	/* dimension of points */
	int dim=3;
	/* number of points */
	int numpoints=0;
	/* array of coordinates for each point */
	coordT *points=0;
	/* True if qhull should free points in qh_freeqhull() or reallocation */
	boolT ismalloc=0;
	/* option flags for qhull, see qh_opt.htm
	 * i: print vertices incident to each facet
	 * Qt: produce triangulated output
	 */
	char flags[]= "qhull i Qt";
	/* output from qh_produce_output()
	 * use NULL to skip qh_produce_output()
	 */
	FILE *outfile= stdout;
	/* error messages from qhull code */
	FILE *errfile= stderr;
	/* 0 if no error from qhull */
	int exitcode;
	/* set by FORALLfacets */
	facetT *facet;
	/* memory remaining after qh_memfreeshort */
	int curlong, totlong;
	
	/* vertexT is a struct containing coordinates etc. */
	vertexT *vertex, **vertexp;
	setT *vertices;
	
	numpoints = verts.size();
	points = new coordT[3 * verts.size()];
	for (int i=0, n=verts.size(); i<n; ++i) {
		points[i*3 + 0] = verts[i][0];
		points[i*3 + 1] = verts[i][1];
		points[i*3 + 2] = verts[i][2];
	}
	
	/* initialize dim, numpoints, points[], ismalloc here */
	exitcode= qh_new_qhull (dim, numpoints, points, ismalloc,
		flags, outfile, errfile);
	if (!exitcode) { /* if no error */ 
		/* 'qh facet_list' contains the convex hull */
		FORALLfacets {
			/* from poly2.c */
			vertices = qh_facet3vertex (facet);
			if (qh_setsize (vertices) == 3) {
				Triangle tri;
				int i = 0;
				FOREACHvertex_(vertices) {
					tri[i++] = qh_pointid(vertex->point);
				}
				tris.push_back(tri);
			}
			qh_settempfree(&vertices);
		}
	}
	qh_freeqhull(!qh_ALL);  
	qh_memfreeshort (&curlong, &totlong);
	if (curlong || totlong)
		fprintf (errfile, "qhull internal warning (main): did not free %d bytes of long memory (%d pieces)\n", 
		totlong, curlong);
	
	delete[] points;
	return tris;
};
