#include "PearsonWeights.h"

#include <cfloat>

// The "training" call
PearsonWeights::PearsonWeights (const std::vector <Eigen::MatrixXd> &raw_features, const std::vector <Eigen::VectorXd> &sample_values, double fraction) {
	int class_index, feature_index;

	class_num = raw_features.size() - 1;

// Class 0 is undefined, and we need at least two defined classes.
	if (class_num < 2) return;

	feature_count = raw_features[1].rows();

	Eigen::VectorXd f_sum (feature_count), f_sum2 (feature_count), cross (feature_count);
	f_sum.setConstant(0);
	f_sum2.setConstant(0);
	cross.setConstant(0);
	double val_sum = 0, val_sum2 = 0, N = 0;
	
	for (class_index = 0; class_index <= class_num; class_index++) {
		const Eigen::MatrixXd &raw_features_ref = raw_features[class_index];
		const Eigen::VectorXd &class_sample_values = sample_values[class_index];
		int n_features = raw_features_ref.rows();
		for (feature_index = 0; feature_index < n_features; feature_index++) {
			f_sum[feature_index]  += raw_features_ref.row(feature_index).sum();
			f_sum2[feature_index] += raw_features_ref.row(feature_index).array().square().sum();
			cross[feature_index]  += (raw_features_ref.row(feature_index).array() * class_sample_values.array()).sum();
		}
		val_sum += class_sample_values.sum();
		val_sum2 += class_sample_values.array().square().sum();
		N += raw_features_ref.cols();
	}
	cross -= ( (f_sum.array() * val_sum) / N ).matrix();
	double val_diff = (val_sum2 - pow(val_sum,2)) / N;
	Eigen::VectorXd denom = ( (f_sum2.array() - f_sum.array().square()) / N) * val_diff;
	
	FeatureWeights = (denom.array() < DBL_EPSILON).select (0, cross.array() / denom.array());
	InitReducedFeatureVecs (fraction);

}

PearsonWeights::~PearsonWeights() {};
