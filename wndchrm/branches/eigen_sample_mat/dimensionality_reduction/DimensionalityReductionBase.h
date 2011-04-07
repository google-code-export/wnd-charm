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


#ifndef __DIMENSIONALITY_REDUCTION_BASE_H__
#define __DIMENSIONALITY_REDUCTION_BASE_H__

#include <assert.h>

// STL stuff
#include <algorithm>
#include <vector>

// Eigen stuff
#include <Eigen/Dense>


// Set up our struct for keeping track of per-feature stuff.
typedef struct {
	int index;        // index into raw_features
	double weight;
} DR_feature_stats_t;

typedef struct DR_sort_by_weight_t {
	bool operator() (DR_feature_stats_t i,DR_feature_stats_t j) { return (j.weight < i.weight);}
} DR_sort_by_weight_func;

class DimensionalityReductionBase {
	public:

	int feature_count;
	int reduced_feature_count;
	int class_num;

	// The destructor should be declared virtual so that the object can be destroyed by calling the base class
	virtual ~DimensionalityReductionBase() {}

	// The "training" call is the cpnstructor
	// stores internal state for assigning weights, retreiving stats
	// The derived classes must have a raw features parameter in the constructor
	// Constructors can't be declared virtual though
	// The constructor should perform as much pre-calculation as possible to minimize work in ProjectFeatures()
	// DimensionalityReductionBase (const std::vector <Eigen::MatrixXd> &raw_features) = 0;

	// This is overridden in classes that derive from DimensionalityReductionFilterBase
	virtual bool isFilter () { return (0); }

	// return projection of raw_features in reduced_features based on internally saved state.
	virtual void ProjectFeatures (Eigen::MatrixXd &projected_features, const Eigen::MatrixXd &raw_features) = 0;

	// These are only valid for classes that derive from DimensionalityReductionFilterBase,
	// but we have to declare these in base because the base class is the one that gets called.
	// The caller is responsible for calling isFilter() to check if these are valid.
	// Is there a better way to do this?
	Eigen::VectorXd FeatureWeights;                       /* weights of all features                    */
	Eigen::VectorXd ReducedFeatureWeights;                /* Weights of the features kept after reduction */
	Eigen::VectorXi ReducedFeatureIndexes;                /* indexes of the features kept after reduction */

	// Filter() is the result of filtering - the reduced_features values should be an unmodified subset of those in the raw_features matrix
	// Algorithms that are not based on filtering should not change the reduced_features matrix
	// For example, a PCA or LDA algorithm is not a filter algorithm, so this call should be a noop.
	virtual inline void Filter (Eigen::MatrixXd &reduced_features, const Eigen::MatrixXd &raw_features) { assert ( isFilter() ); }

	// returns feature statistics for all features.
	// Algorithms not based on individual feature statistics should return a reference to an empty vector
	// Non-filter algorithms (PCA, LDA, etc) should return an empty vector.
	inline const Eigen::VectorXd &GetFeatureWeights () { assert ( isFilter() ); return (FeatureWeights); }
	inline const Eigen::VectorXd &GetReducedFeatureWeights () { assert ( isFilter() ); return (ReducedFeatureWeights); }
	inline const Eigen::VectorXi &GetReducedFeatureIndexes () { assert ( isFilter() ); return (ReducedFeatureIndexes); }

};

class DimensionalityReductionFilterBase : public DimensionalityReductionBase {
public:

	// The destructor should be declared virtual so that the object can be destroyed by calling the base class
	virtual ~DimensionalityReductionFilterBase() {}

	bool isFilter () { return (1); };

	// helper method to sort FeatureWeights
	inline void SortByWeight (std::vector <DR_feature_stats_t> &FeatureStats) {
		assert ( FeatureWeights.size() > 0 );

		for (int feature_index = 0; feature_index < feature_count; feature_index++) {
			FeatureStats[feature_index].index = feature_index;
			FeatureStats[feature_index].weight = FeatureWeights[feature_index];
		}
		DR_sort_by_weight_t DR_sort_by_weight_func;
		sort (FeatureStats.begin(), FeatureStats.end(), DR_sort_by_weight_func);
	}

	// helper method to set up the reduced feature vectors from the full FeatureWeights vector
	virtual inline void InitReducedFeatureVecs (double fraction) {
		std::vector <DR_feature_stats_t> FeatureStats (feature_count);  /* vector of feature indexes and weights, sorted by weight */

		assert ( FeatureWeights.size() > 0 );
		assert ( reduced_feature_count > 0 );
		assert ( feature_count > 0 );

		SortByWeight (FeatureStats);
		reduced_feature_count = (int)floor( (fraction * (double)feature_count) + 0.5 ) + 1;
		ReducedFeatureIndexes.resize (reduced_feature_count);
		ReducedFeatureWeights.resize (reduced_feature_count);
		for (int feature_index = 0; feature_index < reduced_feature_count; feature_index++) {
			ReducedFeatureIndexes[feature_index] = FeatureStats[feature_index].index;
			ReducedFeatureWeights[feature_index] = FeatureStats[feature_index].weight;
		}
	}
	
	// Filter() is the result of filtering - the reduced_features values should be an unmodified subset of those in the raw_features matrix
	virtual inline void Filter (Eigen::MatrixXd &reduced_features, const Eigen::MatrixXd &raw_features) {
		assert ( ReducedFeatureIndexes.size() > 0 );

		if ( raw_features.cols() ) {
			reduced_features.resize (reduced_feature_count,raw_features.cols());
			for (int feature_index = 0; feature_index < reduced_feature_count; feature_index++) {
				reduced_features.row(feature_index) = raw_features.row(ReducedFeatureIndexes[feature_index]);
			}
		}
	}

	
	// return projection of raw_features in reduced_features based on internally saved state.
	// The default method is to multiply the reduced weight vector by each column in the reduced raw_features matrix.
	virtual inline void ProjectFeatures (Eigen::MatrixXd &projected_features, const Eigen::MatrixXd &raw_features) {
		assert ( ReducedFeatureIndexes.size() > 0 );
		assert ( ReducedFeatureWeights.size() > 0 );

		if ( raw_features.cols() ) {
			projected_features.resize (reduced_feature_count,raw_features.cols());
			for (int feature_index = 0; feature_index < reduced_feature_count; feature_index++) {
				projected_features.row(feature_index) = raw_features.row(ReducedFeatureIndexes[feature_index]) * ReducedFeatureWeights[feature_index];
			}
		}
	}

};


#endif // __DIMENSIONALITY_REDUCTION_BASE_H__
