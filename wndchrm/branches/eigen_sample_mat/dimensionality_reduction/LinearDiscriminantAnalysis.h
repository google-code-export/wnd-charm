/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                               */
/*    Copyright (C) 2011 National Institutes of Health                           */
/*                                                                               */
/*    This library is free software; you can redistribute it and/or              */
/*    modify it under the terms of the GNU Lesser General Public                 */
/*    License as published by the Free Software Foundation; either               */
/*    version 2.1 of the License, or (at your option) any later version.         */
/*                                                                               */
/*    This library is distributed in the hope that it will be useful,            */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          */
/*    Lesser General Public License for more details.                            */
/*                                                                               */
/*    You should have received a copy of the GNU Lesser General Public           */
/*    License along with this library; if not, write to the Free Software        */
/*    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  */
/*                                                                               */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                               */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Written by:  Ilya Goldberg <igg [at] nih [dot] gov>                           */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#ifndef __LINEAR_DISCRIMINANT_ANALYSIS_H__
#define __LINEAR_DISCRIMINANT_ANALYSIS_H__

// STL stuff
#include <vector>

// Eigen stuff
#include <Eigen/Dense>

#include "DimensionalityReductionBase.h"


class LinearDiscriminantAnalysis : public DimensionalityReductionBase {
public:

	Eigen::MatrixXd ProjectorMat;
	// The "training" call
	// stores internal state for assigning weights, retreiving stats
	LinearDiscriminantAnalysis (const std::vector <Eigen::MatrixXd> &raw_features);
	~LinearDiscriminantAnalysis ();
	void ProjectFeatures (Eigen::MatrixXd &projected_features, const Eigen::MatrixXd &raw_features) ;

};



#endif // __LINEAR_DISCRIMINANT_ANALYSIS_H__
