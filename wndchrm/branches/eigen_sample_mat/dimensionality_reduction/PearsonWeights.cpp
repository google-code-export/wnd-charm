#include "PearsonWeights.h"

#include <cfloat>

// The "training" call
PearsonWeights::PearsonWeights (const std::vector <Eigen::MatrixXd> &raw_features, const std::vector <Eigen::VectorXd> &sample_values, double fraction) {
	int class_index;

	class_num = raw_features.size() - 1;

// Class 0 is undefined, and we need at least two defined classes.
	if (class_num < 2) return;

	feature_count = raw_features[1].rows();

	Eigen::VectorXd feature_means (feature_count);
	feature_means.setConstant(0);
	double sample_mean = 0, N = 0;
	
	for (class_index = 0; class_index <= class_num; class_index++) {
		const Eigen::MatrixXd &raw_features_ref = raw_features[class_index];
		const Eigen::RowVectorXd &class_sample_values = sample_values[class_index];

		int n_features = raw_features_ref.rows();
		if (! n_features) continue;
		feature_means += raw_features_ref.rowwise().sum();
		sample_mean += class_sample_values.sum();
		N += raw_features_ref.cols();
	}
	sample_mean /= N;
	feature_means /= N;
	
	Eigen::VectorXd f_delta_sum2 (feature_count), delta_cross (feature_count);
	f_delta_sum2.setConstant(0);
	delta_cross.setConstant(0);
	double s_delta_sum2=0;
	for (class_index = 0; class_index <= class_num; class_index++) {
		const Eigen::MatrixXd &raw_features_ref = raw_features[class_index];
		const Eigen::RowVectorXd &class_sample_values = sample_values[class_index];
		int n_features = raw_features_ref.rows();
		if (! n_features) continue;
		for (int sample_index=0; sample_index < raw_features_ref.cols(); sample_index++) {
			double class_sample_delta = class_sample_values[sample_index] - sample_mean;
			Eigen::VectorXd f_deltas = (raw_features_ref.col(sample_index) - feature_means);
			delta_cross  += (f_deltas.array() * class_sample_delta).matrix();
			f_delta_sum2 += f_deltas.array().square().matrix();
			s_delta_sum2 += (class_sample_delta * class_sample_delta);
		}
	}

	Eigen::VectorXd denom = f_delta_sum2.array().sqrt() * sqrt(s_delta_sum2);
	FeatureWeights = (denom.array() < DBL_EPSILON).select (0, delta_cross.array().abs() / denom.array() );

	InitReducedFeatureVecs (fraction);

}

PearsonWeights::~PearsonWeights() {};
