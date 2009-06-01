/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
** Permission to use, copy, modify and distribute this software
** and its documentation for any purpose is hereby granted
** without fee, provided that the above copyright notice appear
** in all copies and that both that copyright notice and this
** permission notice appear in supporting documentation, and
** that the name of EUT not be used in advertising or publicity
** pertaining to distribution of the software without specific,
** written prior permission.  EUT makes no representations about
** the suitability of this software for any purpose. It is provided
** "as is" without express or implied warranty.
** 
** EUT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
** SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL EUT
** BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
** DAMAGES OR ANY DAMAGE WHATSOEVER RESULTING FROM
** LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
** CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
** OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
** OF THIS SOFTWARE.
** 
** 
** Roland Backhouse & Richard Verhoeven.
** Department of Mathematics and Computing Science.
** Eindhoven University of Technology.
**
********************************************************************/
// marker.cc

extern "C" {
#include <stdio.h>
#include "mathpad.h"
}
#include "mathpad.hh"
#include "mark.hh"
#include "marker.hh"
#include "node.hh"

#include "mathpad.icc"

Marker::Marker(Kind k)
{
    _kind=k;
    _next=0;
}

Marker::~Marker()
{
    unlink();
}

void Marker::link()
{
    if (node) {
        _next = node->list;
        node->list = this;
    }
}

void Marker::unlink()
{
    if (node) {
	if (node->list == this) {
	    node->list = _next;
	} else {
	    Marker* pm = node->list;
	    while (pm->_next != this) pm = pm->_next;
	    pm->_next = _next;
	}
	_next = 0;
    }
}

Mark& Marker::operator = (const Mark& m)
{
    unlink();
    node = m;
    pos = m.pos;
    link();
    return *this;
}

Mark& Marker::operator = (const Marker& m)
{
    unlink();
    node = m;
    pos = m.pos;
    link();
    return *this;
}

Node* Marker::operator = (Node* pn)
{
    unlink();
    node = pn;
    link();
    return pn;
}
