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
//         Channels
/////////////////////////////////
// Instead of a string, this should be a channel object
	struct Channel {
		std::string name;

		Channel (std::string &s) { name = s;}
		Channel (const char *s) { name = s;}
	};
// This just returns the string, should return a channel object by string lookup
	static const Channel *getChannelByName (std::string &name);


/////////////////////////////////
//         Transforms
/////////////////////////////////
// Instead of a string, this should be a transform object with an execute() method
	struct Transform {
		std::string name;

		Transform (std::string &s) { name = s;}
		Transform (const char *s) { name = s;}
	};
// This just returns the string, should return a transform object by string lookup
	static const Transform *getTransformByName (std::string &name);


/////////////////////////////////
//      Feature Algorithms
/////////////////////////////////
// This should be a feature algorithm object with an execute() method
	struct FeatureAlgorithm {
		std::string name;
		int n_features;

		FeatureAlgorithm () : name(""), n_features(1) { }
		FeatureAlgorithm (std::string &s,int i) { name = s; n_features = i;}
		FeatureAlgorithm (const char *s,int i) { name = s; n_features = i;}
	};
// This returns an iterator to the algorithm map by string lookup
	static const FeatureAlgorithm *getFeatureAlgorithmByName (std::string &name);


/////////////////////////////////
//        Feature Groups
/////////////////////////////////
	struct FeatureGroup {
		std::string name;
		const FeatureAlgorithm *algorithm;
		const Channel *channel;
		std::vector<Transform const *> transforms; // these are in order of application

		FeatureGroup () : algorithm(NULL), channel(NULL) {};
		FeatureGroup (std::string &s, const FeatureAlgorithm *f, const Channel *c, std::vector<Transform const *> t) {
			name = s; algorithm = f; channel = c; transforms = t;
		}
	};
// This will store a new group if the name doesn't exist.
// The returned pointer is to an iterator into the static group map
	static const FeatureGroup *getGroupByName (std::string &name);


/////////////////////////////////
//          Features
/////////////////////////////////
	struct Feature {
		std::string name;
		const FeatureGroup *group;
		int index; // within group

		Feature () : group(NULL), index(-1) {};
		Feature (std::string &s, const FeatureGroup *g, int i) { name = s; group = g; index = i;}
	};
	static const Feature *getFeatureByName (const char *featurename_in);

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
	typedef MAP<std::string, Channel *> cnm_t;
	static cnm_t  channels_;

	typedef MAP<std::string, Transform *> tnm_t;
	static tnm_t  transforms_;

	typedef MAP<std::string, FeatureAlgorithm *> fam_t;
	static fam_t  feature_algorithms_;

	typedef MAP<std::string, FeatureGroup *> fgnm_t;
	static fgnm_t feature_groups_;

	typedef MAP<std::string, Feature *> fnm_t;
	static fnm_t  features_;

	
};

#endif // __FEATURE_INFO_HPP__
