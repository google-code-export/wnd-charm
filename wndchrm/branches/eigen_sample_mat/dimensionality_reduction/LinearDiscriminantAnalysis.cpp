#include "LinearDiscriminantAnalysis.h"

#include <cfloat>

using namespace Eigen;

template<typename Scalar>
static bool pinv(const Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> &a,
	Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> &result,
	double epsilon = std::numeric_limits<Scalar>::epsilon())
{
	if(a.rows()<a.cols())
		return false;
	Eigen::JacobiSVD< Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > svd (a, ComputeThinU | ComputeThinV );

	Scalar tolerance = epsilon * std::max(a.cols(), a.rows()) * svd.singularValues().cwiseAbs().maxCoeff();

	result = svd.matrixV() * (svd.singularValues().array().abs() > tolerance).select(svd.singularValues().
		array().inverse(), 0).matrix().asDiagonal() * svd.matrixU().adjoint();

	return true;
}



// LinearDiscriminantAnalysis
//
// This function is a transliteration from matlab.  The matlab code is included in the comments.
// The "training" call is the constructor
LinearDiscriminantAnalysis::LinearDiscriminantAnalysis (const std::vector <Eigen::MatrixXd> &class_features) {

// function [P,v] = compute_LDA(X,class_vec),
// X: Features X n-Samples
// class_features is a vector of per-class X matrixes (i.e. one [features X samples] matrix for each class)

// function [P,v] = compute_LDA(X,class_vec),
// %~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// % LDA, preparotary steps: compute scatter matrices...
// % Number of sigs
// Nsig = size(X,1);
	size_t Nsig = class_features[0].rows();
// In wndchrm, class number is 1-based. n_classes = max_class-1;
	size_t max_class = class_features.size();
// 	cout << "max_class: " << max_class << "\n";

// % Mean feature value over all samples ( = Features X 1 )
// The parameter is an array of feature matrixes, so we sum row-wise into a vector, then divide by the total columns
// mm = mean(X,2);
	VectorXd mm = VectorXd::Zero(Nsig);
	size_t i,n_samples=0;
	for (i = 1; i < max_class; i++) {
		mm += class_features[i].rowwise().sum();
		n_samples += class_features[i].cols();
	}
	mm /= n_samples;
// 	cout << "mm:\n" << mm << "\n";
	

// Number of unique classes (1 x n_classes)
// We don't need to re-organize the feature vector because this is done already
// Ncla is not used as it has a slightly different meaning in wndchrm.
// Instead we use max_class, which is the highest valid class index.
// ucla = unique(class_vec);
// Ncla = length(ucla);
// initialize a Nsig X Nsig matrix of zeroes
// Sb = zeros(Nsig,Nsig);
	MatrixXd Sb = MatrixXd::Zero(Nsig,Nsig);
	MatrixXd Sw = MatrixXd::Zero(Nsig,Nsig);

// % 1..number of classes.  currentClass is 1 X n_class_samples; Nk is # of features samples in class
// for k = 1:Ncla, currentClass = find(class_vec == ucla(k)); Nk = length(currentClass);
	for (i = 1; i < max_class; i++) {

//  Xk = X(:,currentClass);
//	N.B.: Xk is simply class_features[i] because of the pre-sorted class_features parameter;
		size_t Nk = class_features[i].cols();

// 	% mk: centroid for class 1
//  mk = mean( Xk,2 );
		VectorXd mk;
		mk = class_features[i].rowwise().mean();
// 		cout << "mk:\n" << mk << "\n";

//  delta_m = mm - mk;
		VectorXd delta_m = mm - mk;

//  Swk(:,:,k) = ( Xk - mk*ones(1,size(Xk,2)) ) * ( Xk - mk*ones(1,size(Xk,2)) )';
		RowVectorXd ones = RowVectorXd::Ones(Nk);
		Sw += (  class_features[i] - mk * ones ) * (  class_features[i] - mk * ones ).transpose();
// 		MatrixXd cmo = class_features[i] - mk * RowVectorXd::Ones(Nk);
// 		Sw += cmo * cmo.transpose();

//  Sb = Sb + Nk * delta_m * delta_m';
		Sb += Nk * delta_m * delta_m.transpose();

// end % ss
	}

// We used an accumulator in the loop, so we don't need a sum here
// Sw = sum(Swk,3); clear Swk;

// 
// % LDA, Rayleigh Quotient...
// % pinv: Moore-Penrose pseudoinverse of matrix
// pind() is implemented below using Eigen's JacobiSVD
// RQ = pinv(Sw)*Sb;
 	MatrixXd Sw_pinv (Nsig,Nsig);
 	pinv (Sw, Sw_pinv);

// need to check wether the function returned true or false (false if didn't converge)
 	MatrixXd RQ = Sw_pinv * Sb;

// % Compute singular values characterizing argmax of the Quotient...
// Need to check wether this converged or not...
// [U,S,V] = svd(RQ','econ'); clear RQ;
	JacobiSVD<MatrixXd> svd(RQ.transpose(), ComputeThinU | ComputeThinV);

// 
// % LDA, take K significant values...
// K = Ncla-1; u = U(:,1:K); v = V(:,1:K); s = S(1:K,1:K);
// In wndchrm, class number is 1-based. n_classes = max_class-1;
	size_t K = max_class-2;

// We're not using u or s for anything, so don't bother computing them
// Note that to get Matlab's S, we need to construct a matrix that uses svd.singularValues() as a diagonal:
//	MatrixXd s = MatrixXd::Zero(K,K); s.diagonal() = svd.singularValues().topRows(K);	
	ProjectorMat = svd.matrixV().leftCols(K);

// 
// % LDA, project feature space onto K dimensions...
// Here we're doing it once per class.
// P = (X'*v)';
// 	for (i = 1; i < max_class; i++) {
// 		MatrixXd P = (class_features[i].transpose() * v).transpose();
// //		cout << "P for class " << i << ":\n" << P << "\n\n";
// 	}

// end % eofunc
}

// This function projects raw features into a LDA space
inline void LinearDiscriminantAnalysis::ProjectFeatures (Eigen::MatrixXd &projected_features, const Eigen::MatrixXd &raw_features) {
		projected_features = (raw_features.transpose() * ProjectorMat).transpose();
}


LinearDiscriminantAnalysis::~LinearDiscriminantAnalysis() {};
