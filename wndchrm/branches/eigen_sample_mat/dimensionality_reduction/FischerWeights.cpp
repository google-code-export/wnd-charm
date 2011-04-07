#include "FischerWeights.h"

#include <cfloat>

// The "training" call
FischerWeights::FischerWeights (const std::vector <Eigen::MatrixXd> &raw_features, double fraction) {
	int class_index;

	class_num = raw_features.size() - 1;

// Class 0 is undefined, and we need at least two defined classes.
	if (class_num < 2) return;

	feature_count = raw_features[1].rows();

	Eigen::MatrixXd class_mean (feature_count,class_num),class_var (feature_count,class_num);
	Eigen::MatrixXd class_delta;
	Eigen::VectorXd mean_class_means, mean_class_var, mean_inter_class_var;
	for (class_index = 1; class_index <= class_num; class_index++) {
		const Eigen::MatrixXd &raw_features_ref = raw_features[class_index];
		class_mean.col(class_index-1) = raw_features_ref.rowwise().mean();
		class_var.col(class_index-1) = (raw_features_ref.colwise() - class_mean.col(class_index-1)).array().square().matrix().rowwise().mean();
	}

	mean_class_means = class_mean.rowwise().mean();

	mean_class_var = ( class_mean.colwise() - mean_class_means ).array().square().matrix().rowwise().sum();
	mean_class_var /= class_num-1;

	mean_inter_class_var = class_var.rowwise().mean();
	mean_inter_class_var = (mean_inter_class_var.array() < DBL_EPSILON).select (DBL_EPSILON, mean_inter_class_var);

	FeatureWeights = mean_class_var.array() / mean_inter_class_var.array();

	InitReducedFeatureVecs (fraction);

}

FischerWeights::~FischerWeights() {};
