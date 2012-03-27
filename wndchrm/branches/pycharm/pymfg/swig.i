%{
/****************************************************************************/
/*  mfGraph Library Python bindings                                         */
/*  Copyright (c) 2003 Michael Foetsch                                      */
/*                                                                          */
/*  This library is free software; you can redistribute it and/or           */
/*  modify it under the terms of the GNU Lesser General Public              */
/*  License as published by the Free Software Foundation; either            */
/*  version 2.1 of the License, or (at your option) any later version.      */
/*                                                                          */
/*  This library is distributed in the hope that it will be useful,         */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU        */
/*  Lesser General Public License for more details.                         */
/*                                                                          */
/*  You should have received a copy of the GNU Lesser General Public        */
/*  License along with this library; if not, write to the Free Software     */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA */
/*                                                                          */
/*  Contact:    foetsch@yahoo.com                                           */
/*              http://mfgraph.sourceforge.net/                             */
/****************************************************************************/

#include "cmatrix.h"
#include "wndchrm_error.h"
#include "FeatureAlgorithms.h"
#include "FeatureTransforms.h"
%}

%module pymfg


%include "cmatrix.i"
%include "wndchrm_error.i"
%include "FeatureAlgorithms.i"
%include "FeatureTransforms.i"
