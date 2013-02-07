#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "FeatureNames.hpp"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

// Storage for class statics
FeatureNames::cnm_t  FeatureNames::channels_;
FeatureNames::tnm_t  FeatureNames::transforms_;
FeatureNames::fam_t  FeatureNames::feature_algorithms_;
FeatureNames::fgnm_t FeatureNames::feature_groups_;
FeatureNames::fnm_t  FeatureNames::features_;
FeatureNames::ofnm_t FeatureNames::old_features_;
// initialization pre-main
const bool initOldFeatureNameLookup_ = FeatureNames::initOldFeatureNameLookup ();
const bool initFeatureAlgorithms_ = FeatureNames::initFeatureAlgorithms ();

// This defines a big C string constant (oldFeatureNamesFileStr) with tab-delimited old_feature new_feature lines.
// Its defined this way because doing map declarations on the stack takes forever to compile,
// blows up memory during compilation with optimization, and results in a monstrously huge object file 5x bigger than the rest of the library
#include "OldFeatureNamesFileStr.h"



// testing only
// #define CYCLES 100
// 
// typedef struct {
// 	std::string name;
// 	const FeatureNames::FeatureInfo *feature_info;
// 	int n_hits;
// } featuregroup_stats_t;
// struct sort_by_n_features_t {
// 	bool operator() (featuregroup_stats_t i,featuregroup_stats_t j) { return (i.feature_info->group->algorithm->n_features<j.feature_info->group->algorithm->n_features);}
// } sort_by_n_features;
// // testing only
// 
// int main () {
// 	int i,j,n_featnames;
// 	FeatureNames::ofnm_t::const_iterator ofnm_it;
// 	std::vector<std::string> featurenames;
// 
// 	featurenames.reserve(FeatureNames::old_features_.size());
// 	for(ofnm_it = FeatureNames::old_features_.begin(); ofnm_it != FeatureNames::old_features_.end(); ++ofnm_it ) {
// 		featurenames.push_back( ofnm_it->first );
// 	}
// 	n_featnames = featurenames.size();
// 
// 	std::set<std::string> fgs;
// 	std::set<std::string>::iterator fgs_it;
// 	std::vector<featuregroup_stats_t> fgsv;
// 	std::vector<featuregroup_stats_t>::iterator fgsv_it;
// 	featuregroup_stats_t featuregroup_stats;
// 	FeatureNames::FeatureInfo const *featureinfo;
// 	for (i = 0; i< n_featnames; i++) {
// 		featureinfo = FeatureNames::getFeatureInfoByName ( featurenames[i].c_str() );
// 		fgs_it = fgs.find(featureinfo->group->name);
// 		if (fgs_it == fgs.end()) {
// 			featuregroup_stats.name = featureinfo->group->name;
// 			featuregroup_stats.feature_info = featureinfo;
// 			featuregroup_stats.n_hits = 0;
// 			fgs.insert(featureinfo->group->name);
// 			fgsv.push_back (featuregroup_stats);
// 		}
// 	}
// 
// 	sort (fgsv.begin(), fgsv.end(), sort_by_n_features);
// 	for(fgsv_it = fgsv.begin(); fgsv_it != fgsv.end(); ++fgsv_it ) {
// 		printf ("%s: %d\n",fgsv_it->name.c_str(), fgsv_it->feature_info->group->algorithm->n_features);
// 	}
// 
// 
// 
// // 	fgnm_t *fgnm = featureGroups();
// // 	fgnm_it_t fgnm_it;
// // 	for(fgnm_it = (*fgnm).begin(); fgnm_it != (*fgnm).end(); ++fgnm_it ) {
// // 		printf ("%s: %d\n",fgnm_it->second.name.c_str(), fgnm_it->second.n_features);
// // 	}
// 
// 
// 
// 	int found=0;
// 	int missed=0;
// 	double cpu_secs;
// 	clock_t start, end;
// 
// // time parse feature - found
// 	found=0;
// 	missed=0;
// 	start = clock();
// 	for (j = 0; j < CYCLES; j++) {
// 		for (i = 0; i< n_featnames; i++) {
// 			if (! FeatureNames::getFeatureInfoByName ( featurenames[i].c_str() ) ) missed++;
// 			else found++;
// 		}
// 	}
// 	end = clock();
// 	cpu_secs = ((double) (end - start)) / CLOCKS_PER_SEC;
// 	printf ("       +parse %9d lookups %9d misses %9.2g secs, %9.0f lookups/sec\n",found+missed,missed,cpu_secs,(double)(found+missed)/cpu_secs);
// 
// 
// // time parse feature - nonexistant
// 	found=0;
// 	missed=0;
// 	start = clock();
// 	for (j = 0; j < CYCLES; j++) {
// 		for (i = 0; i< n_featnames; i++) {
// 		// worst- case can't find algorithm
// 		//	if ( !FeatureNames::getFeatureInfoByName ( "CoOcMat_MaximalCorrelationCoefficientDif_ChebyshevFFT MaxCorrCoef () [123]" ) ) missed++;
// 		// second-worst - malformed invalid algorithm: no '()'
// 			if ( !FeatureNames::getFeatureInfoByName ( "CoOcMat_MaximalCorrelationCoefficientDif_ChebyshevFFT MaxCorrCoef [123]" ) ) missed++;
// 		// best-case invalid name - no []
// 		//	if ( !FeatureNames::getFeatureInfoByName ( "CoOcMat_MaximalCorrelationCoefficientDif_ChebyshevFFT MaxCorrCoef" ) ) missed++;
// 			else found++;
// 		}
// 	}
// 	end = clock();
// 	cpu_secs = ((double) (end - start)) / CLOCKS_PER_SEC;
// 	printf ("       -parse %9d lookups %9d misses %9.2g secs, %9.0f lookups/sec\n",found+missed,missed,cpu_secs,(double)(found+missed)/cpu_secs);
// 
// }

/*
New-style feature names follow this style:
  Zernike Coefficients (Wavelet (Edge ())) [21]
  Zernike Coefficients (Wavelet ()) [21]
  Zernike Coefficients () [21]
Algorithm name followed by a set of nested transforms in parentheses, then feature index within the group in square brackets.
The inner-most parentheses contain an optional channel label
White-space is not used as a delimiter - only '(',')','[' and ']'. Leading and trailing whitespace is eliminated from transforms and algorithm names.
The parentheses are *required* to parse transforms. In their absence, the entire feature name (other than square brackets) is taken as the algorithm name.
The square brackets are required for indexes.  In their absence, an index of 0 is assumed.
The index is 0-based.
The transforms are applied in reverse order to what is listed, as expected for the nested representation.
In the example above, Edge transform first, then Wavelet, then Zernike coefficients on the Wavelet.
The feature and group names reported in .name fields are normalized for whitespace as in the example above.
*/

const FeatureNames::FeatureInfo *FeatureNames::getFeatureInfoByName (const char *featurename_in) {
	if (! (featurename_in && *featurename_in) ) return (NULL);
	fnm_t::const_iterator fnm_it = features_.find(featurename_in);
	if (fnm_it != features_.end()) return (fnm_it->second);

	const std::string *featurename_old;
	std::string featurename;
	featurename_old = oldFeatureNameLookup (featurename_in);
	if (featurename_old) featurename.assign (*featurename_old);
	else featurename.assign (featurename_in);

// parse out the group index
	int index = -1;
	size_t found_left = featurename.find_last_of ('[');
	size_t found_right = featurename.find_last_of (']');
	if (found_left == std::string::npos || found_right == std::string::npos || found_right-found_left-1 < 1)
		index = -1;
	else
		index = atoi ( (featurename.substr (found_left+1,found_right-found_left-1)).c_str() );

// parse out the group name
	size_t found;
	std::string groupname;
	groupname = featurename.substr (0,found_left);
	// clean trailing whitespace
	found=groupname.find_last_not_of(" \t\f\v\n\r");
	if (found != std::string::npos)
		groupname.erase(found+1);

	const FeatureGroup *featuregroup = getGroupByName (groupname);
	if (!featuregroup) return (NULL); // This should only fail for memory allocation failure

	// For now, if we got an invalid (unknown) index, assume that its 0.
	if (index < 0) index = 0;

	// note that the feature name is normalized for whitespace
	featurename = featuregroup->name;

	if (index >= 0) {
		char buf[64];
		sprintf(buf,"%d",index);
		featurename += " [";
		featurename += buf;
		featurename += "]";
	}
	// For unknown featuregroups, make sure that n_features is always big enough to accomodate the index (if any)
	if (index > featuregroup->algorithm->n_features) {
	// bypass static constraint
		int *index_p = (int *)&(featuregroup->algorithm->n_features);
		*index_p = index;
	}

	FeatureInfo *featureinfo = new FeatureInfo (featurename, featuregroup, index);

	features_[featurename_in] = featureinfo;
	return (featureinfo);
}

// This returns an iterator to the algorithm map by string lookup
const FeatureNames::FeatureAlgorithm *FeatureNames::getFeatureAlgorithmByName (std::string &name) {
	fam_t::const_iterator fam_it = feature_algorithms_.find(name);
	FeatureAlgorithm *algorithm = NULL;
	
	if (fam_it == feature_algorithms_.end()) {
		algorithm = new FeatureAlgorithm (name,1);
		feature_algorithms_[name] = algorithm;
	} else {
		algorithm = fam_it->second;
	}
	
	return (algorithm);
}


// This should return a channel object by string lookup
const FeatureNames::Channel *FeatureNames::getChannelByName (std::string &name) {
	cnm_t::const_iterator cnm_it = channels_.find(name);
	Channel *channel = NULL;

	if (cnm_it == channels_.end()) {
		channel = new Channel (name);
		channels_[name] = channel;
	} else {
		channel = cnm_it->second;
	}

	return (channel);
}

// This should return a transform object by string lookup
const FeatureNames::Transform *FeatureNames::getTransformByName (std::string &name) {
	tnm_t::const_iterator tnm_it = transforms_.find(name);
	Transform *transform = NULL;

	if (tnm_it == transforms_.end()) {
		transform = new Transform (name);
		transforms_[name] = transform;
	} else {
		transform = tnm_it->second;
	}

	return (transform);
}


const FeatureNames::FeatureGroup *FeatureNames::getGroupByName (std::string &name) {
	fgnm_t::const_iterator fgnm_it = feature_groups_.find(name);
	if (fgnm_it != feature_groups_.end()) return (fgnm_it->second);

	FeatureGroup *featuregroup=NULL;
	size_t found;
	
//printf ("groupname cache miss\n");

// parse out algorithm name: everything up to the first '(' without trailing whitespace
	std::string algorithmname;
	FeatureAlgorithm *algorithm;
	size_t found_parens = name.find_first_of ('(');
	algorithmname.assign (name,0,found_parens);
	// clean trailing whitespace
	found=algorithmname.find_last_not_of(" \t\f\v\n\r");
	if (found != std::string::npos)
		algorithmname.erase(found+1);

	algorithm = (FeatureAlgorithm *) getFeatureAlgorithmByName (algorithmname);
	if (!algorithm) return (NULL); // This should only fail on memory allocation
//printf ("ref string c_str: [%s]\n",featuregroup.name.c_str());

// parse out the transforms - separated by '('
	size_t found_trans_s;
	size_t found_trans_e;
	std::string transform_name;
	std::vector<Transform const *> transforms;
	const Transform *transform;

	found_trans_s = name.find_first_not_of(" ()",found_parens+1);
	if (found_trans_s != std::string::npos) found_trans_e = name.find_first_of('(',found_trans_s+1);
	else (found_trans_e = std::string::npos);
	while (found_trans_e != std::string::npos) {
		transform_name.assign (name.substr (found_trans_s,found_trans_e-found_trans_s));
	// clean trailing whitespace
		found=transform_name.find_last_not_of(" \t\f\v\n\r");
		if (found != std::string::npos) {
			transform_name.erase(found+1);
			transform = getTransformByName(transform_name);
			if (transform) transforms.push_back ( transform );
			else return (NULL);
		}
		found_trans_s = name.find_first_not_of(" ()",found_trans_e+1);
		if (found_trans_s != std::string::npos) found_trans_e = name.find_first_of('(',found_trans_s+1);
		else (found_trans_e = std::string::npos);
	}
// The vector holds the transforms in application order, which opposite of left-to-right read order.
	std::reverse(transforms.begin(),transforms.end());

// Parse the channel
// If there is a channel specified, its in the inner parens.
// found_trans_s points at its first char, and found_trans_e points at npos
	std::string channel_name;
	Channel *channel = NULL;
	if (found_trans_s != std::string::npos && (found_trans_e = name.find_first_of(')',found_trans_s+1)) != std::string::npos ) {
		channel_name.assign (name.substr (found_trans_s,found_trans_e-found_trans_s));
		found=channel_name.find_last_not_of(" \t\f\v\n\r");
		if (found != std::string::npos)	{
			channel_name.erase(found+1);
			channel = (Channel *)getChannelByName(channel_name);
	// Empty parens - or not closed
		} else {
			channel = NULL;
		}
// Empty parens
	} else {
		channel = NULL;
	}

// Normalize the whitespace in the group name
	std::string name_norm = algorithm->name;
	size_t i;
	name_norm += " (";
	for (i = transforms.size(); i > 0  ; i--) {
		name_norm += transforms[i-1]->name + " (";
	}
	if (channel) name_norm += channel->name;
	name_norm += ")";
	for (i = 0; i < transforms.size(); i++) name_norm += ")";

// end of validation checks
	featuregroup = new FeatureGroup (name, algorithm, channel, transforms);
	featuregroup->name = name_norm;
	featuregroup->algorithm = algorithm;
	featuregroup->channel = channel;
// These need to go in backwards from how they were read.
	featuregroup->transforms = transforms;


// 	printf ("%s: [%s]",name.c._str(),featuregroup.algorithm.c_str());
// 	if (featuregroup.transforms.size()) printf (" [%s]",featuregroup.transforms[featuregroup.transforms.size()-1].c_str());
// 	for (int i= featuregroup.transforms.size()-2; i >= 0 ; i--) printf ("->[%s]",featuregroup.transforms[i].c_str());
// 	printf ("[%d]\n",index);


	feature_groups_[name] = featuregroup;
	return (featuregroup);
}


const std::string *FeatureNames::oldFeatureNameLookup (const char *oldFeatureName) {
	ofnm_t::const_iterator ofnm_it = old_features_.find(oldFeatureName);

	if (ofnm_it == old_features_.end()) return (NULL);
	else return (&(ofnm_it->second));
}



const bool FeatureNames::initFeatureAlgorithms() {

	if (!feature_algorithms_.empty()) return (true);
	feature_algorithms_["Chebyshev Coefficients"]         = new FeatureAlgorithm ("Chebyshev Coefficients",         31);
	feature_algorithms_["Chebyshev-Fourier Coefficients"] = new FeatureAlgorithm ("Chebyshev-Fourier Coefficients", 31);
	feature_algorithms_["Color Histogram"]                = new FeatureAlgorithm ("Color Histogram",                18);
	feature_algorithms_["Comb Moments"]                   = new FeatureAlgorithm ("Comb Moments",                   47);
	feature_algorithms_["Edge Features"]                  = new FeatureAlgorithm ("Edge Features",                  27);
	feature_algorithms_["Fractal Features"]               = new FeatureAlgorithm ("Fractal Features",               19);
	feature_algorithms_["Gabor Textures"]                 = new FeatureAlgorithm ("Gabor Textures",                  6);
	feature_algorithms_["Haralick Textures"]              = new FeatureAlgorithm ("Haralick Textures",              27);
	feature_algorithms_["Multiscale Histograms"]          = new FeatureAlgorithm ("Multiscale Histograms",          23);
	feature_algorithms_["Object Features"]                = new FeatureAlgorithm ("Object Features",                33);
	feature_algorithms_["Pixel Intensity Statistics"]     = new FeatureAlgorithm ("Pixel Intensity Statistics",      4);
	feature_algorithms_["Radon Coefficients"]             = new FeatureAlgorithm ("Radon Coefficients",             11);
	feature_algorithms_["Tamura Textures"]                = new FeatureAlgorithm ("Tamura Textures",                 5);
	feature_algorithms_["Zernike Coefficients"]           = new FeatureAlgorithm ("Zernike Coefficients",           71);
	return (true);
}

// N.B.: not a class or object method.
void parseFeatureNameMap(char *buffer, FEATURENAMES_MAP<std::string,std::string> &feature_name_map ) {
	char *p = buffer;
	enum {st_eol, st_leading_space, st_ignore_line, st_key, st_val} state = st_eol, new_state = st_eol;
	std::string key, val;

	while (*p) {
		key = val = "";
		while (*p && state != st_key) {
			if (*p == '\n') new_state = st_eol;
			else if ( (state == st_eol || state == st_leading_space) && isspace (*p)) new_state = st_leading_space;
			else if ( (state == st_eol || state == st_leading_space) && !isalpha (*p)) new_state = st_ignore_line;
			else if (state == st_eol || state == st_leading_space) new_state = st_key;
		
			state = new_state;
			if (state != st_key) p++;
		}

		// key is everything up until the first tab.
		while (*p && state != st_val) {
			if (*p == '\t') new_state = st_val;
			else key += *p;

			state = new_state;
			if (state != st_val) p++;
		}
		
		// consume whitespace
		while (*p && isspace (*p)) p++;

		// value is everything until '\n'
		while (*p && state != st_eol) {
			if (*p == '\n') new_state = st_eol;
			else val += *p;

			state = new_state;
			if (state != st_eol) p++;
		}
		if (key.length()) feature_name_map[key] = val;
	}

}


const bool FeatureNames::initOldFeatureNameLookup () {
	
	if (!old_features_.empty()) return (true);
	parseFeatureNameMap (oldFeatureNamesFileStr, old_features_);
	return (true);
}

