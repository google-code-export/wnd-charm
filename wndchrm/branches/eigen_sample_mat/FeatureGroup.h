#ifndef __FEATUREGROUPS_H__
#define __FEATUREGROUPS_H__

#include "MatrixMap.h"
#include "wndchrm_error.h"

//=====================================================================
/*!
 * Feature Groups
 */
class Transform;
class FeatureAlgorithm;
class ImageMatrix;
class Channel;

class FeatureGroup {
	public:
		std::string name;
		//const FeatureAlgorithm* algorithm;
		FeatureAlgorithm* algorithm;
		//const Channel* channel;
		Channel* channel;
		//std::vector<Transform const *> transforms; // these are in order of application
		std::vector<Transform *> transforms; // these are in order of application
		FeatureGroup () : algorithm(NULL), channel(NULL) {};
		//FeatureGroup (string &s, const FeatureAlgorithm *f, const Channel *c, std::vector<Transform const *> t) {
		FeatureGroup (std::string &s, FeatureAlgorithm *f, Channel *c, std::vector<Transform *> t) {
			name = s; algorithm = f; channel = c; transforms = t;
		}
		void          print_info() const;
		WNDCHRM_ERROR get_name( std::string& out_str );
		WNDCHRM_ERROR calculate_coefficients( MatrixMap& saved_pixel_planes, 
		                                      std::vector<double> &coeffs );
		
};

#endif
