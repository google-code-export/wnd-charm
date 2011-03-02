#ifndef __FEATURE_INFO_HPP__
#define __FEATURE_INFO_HPP__

#include <string>
#include <vector>
#include <map>
#include <set>

/*
map and unordered_map comparison:
std::tr1::unordered_map:
       +parse    502400 lookups         0 misses      0.46 secs,   1091977 lookups/sec
       -parse    502400 lookups    502400 misses       2.2 secs,    229342 lookups/sec
std::map:
       +parse    502400 lookups         0 misses      0.82 secs,    610048 lookups/sec
       -parse    502400 lookups    502400 misses       2.4 secs,    205706 lookups/sec
*/

#include "config.h"
#ifdef HAVE_UNORDERED_MAP
# include <unordered_map>
# define MAP std::unordered_map
#elsif HAVE_TR1_UNORDERED_MAP
# include <tr1/unordered_map>
# define MAP std::tr1::unordered_map
#else
# define MAP std::map
#endif


class FeatureNames {
public:
/////////////////////////////////
//         Transforms
/////////////////////////////////
// Instead of a string, this should be a transform object with an execute() method
	typedef std::string transform_t;
// This just returns the string, should return a transform object by string lookup
	static const transform_t *getTransformByName (std::string &name);


/////////////////////////////////
//      Feature Algorithms
/////////////////////////////////
// This should be a feature algorithm object with an execute() method
	typedef struct {
	std::string name;
	int n_features;
} feature_algorithm_t;
// This returns an iterator to the algorithm map by string lookup
	static const feature_algorithm_t *getFeatureAlgorithmByName (std::string &name);


/////////////////////////////////
//        Feature Groups
/////////////////////////////////
	typedef struct {
		std::string name;
		feature_algorithm_t *algorithm;
		int n_features;
		std::vector<transform_t const *> transforms; // these are in order of application
	} featuregroup_t;
// This will store a new group if the name doesn't exist.
// The returned pointer is to an iterator into the static group map
	static const featuregroup_t *getGroupByName (std::string &name);


/////////////////////////////////
//          Features
/////////////////////////////////
	typedef struct {
		std::string name;
		const featuregroup_t *group;
		int index; // within group
	} featureinfo_t;
	static const featureinfo_t *getFeatureByName (const char *featurename_in);

/////////////////////////////////
// Old-style feature name lookup
/////////////////////////////////
	static const std::string *oldFeatureNameLookup (const char *oldFeatureName);

// These have to be public and static in order to be called pre-main without an object.
// This is done in FeatureNames.cpp in the global scope - outside of any functions/methods.
	static const bool initFeatureAlgorithms();
	static const bool initOldFeatureNameLookup();

// TEMPORARY used for testing by main() in FeatureNames.cpp - move to private.
	typedef MAP<std::string,std::string> ofnm_t;
	static ofnm_t old_features_;

private:
////////////////////////////////////////
// Private static object caches
////////////////////////////////////////
	typedef MAP<std::string,transform_t *> tnm_t;
	static tnm_t  transforms_;

	typedef MAP<std::string,feature_algorithm_t> fam_t;
	static fam_t  feature_algorithms_;

	typedef MAP<std::string,featuregroup_t *> fgnm_t;
	static fgnm_t feature_groups_;

	typedef MAP<std::string,featureinfo_t *> fnm_t;
	static fnm_t  features_;

	
};

#endif // __FEATURE_INFO_HPP__
