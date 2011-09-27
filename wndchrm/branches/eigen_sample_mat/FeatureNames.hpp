#ifndef __FEATURE_NAMES_HPP__
#define __FEATURE_NAMES_HPP__

#include <string>
#include <vector>
#include <set>
#include "config.h"
#include "MAP.h"  // Defines the macro MAP which decides whether to #include
                  // <unordered_map>, <tr1/unordered_map>, or just plain ol <map>
//#include "transforms.h"
#include "Channel.h"
#include "FeatureGroup.h"
#include "cmatrix.h"


/*
map and unordered_map comparison:
std::tr1::unordered_map:
       +parse    502400 lookups         0 misses      0.46 secs,   1091977 lookups/sec
       -parse    502400 lookups    502400 misses       2.2 secs,    229342 lookups/sec
std::map:
       +parse    502400 lookups         0 misses      0.82 secs,    610048 lookups/sec
       -parse    502400 lookups    502400 misses       2.4 secs,    205706 lookups/sec
*/

class Transform;
class FeatureAlgorithm;
class ImageMatrix;

//=====================================================================
/*!
 * Features
 */
class FeatureInfo {
	public:

		std::string name;
		//const FeatureGroup *group;
		FeatureGroup *group;
		int index; // within group
 
		FeatureInfo () : group(NULL), index(-1) {};
		// FeatureInfo (std::string &s, const FeatureGroup *g, int i) { name = s; group = g; index = i;}
		FeatureInfo (std::string &s, FeatureGroup *g, int i) { name = s; group = g; index = i;}
		// FeatureInfo (const FeatureGroup* g, int i) { group = g; index = i; }
		FeatureInfo (FeatureGroup* g, int i) { group = g; index = i; }
		int get_name ( string& out_str );
		void print_info() const;
};

class FeatureNames {
public:
	~FeatureNames() {instanceFlag = false; delete pInstance;};
 
	static FeatureNames* get_instance();

	//! This just returns the string, should return a channel object by string lookup
	// CEC_const const Channel *getChannelByName (std::string &name);
	Channel *getChannelByName (std::string &name);

	//! This just returns the string, should return a transform object by string lookup
	// CEC_const const Transform *getTransformByName (std::string &name);
	Transform *getTransformByName (std::string &name);

	//! This returns an iterator to the algorithm map by string lookup
	// CEC_const const FeatureAlgorithm *getFeatureAlgorithmByName (std::string &name);
	FeatureAlgorithm *getFeatureAlgorithmByName (std::string &name);

	//! This will store a new group if the name doesn't exist.
	//! The returned pointer is to an iterator into the static group map
	// CEC_const const FeatureGroup *getGroupByName (const char* name);
	FeatureGroup *getGroupByName (const char* name);
	// CEC_const const FeatureGroup *getGroupByName (std::string &name);
	FeatureGroup *getGroupByName (std::string &name);

	// CEC_const const FeatureInfo *getFeatureInfoByName (const char *featurename_in);
	FeatureInfo *getFeatureInfoByName (const char *featurename_in);

	//! Old-style feature name lookup
	const std::string *oldFeatureNameLookup (const char *oldFeatureName);

  // These have to be public and static in order to be called pre-main without an object.
  // This is done in FeatureNames.cpp in the global scope - outside of any functions/methods.
	const bool initFeatureAlgorithms();
	const bool initOldFeatureNameLookup();

	//! List of FeatureGroups that comprise the Long chain
	//vector<FeatureGroup> 

	//REGISTRATION METHODS
	int register_transform( string &transform_name, Transform * BT_itf );
	int register_algorithm( string &alg_name, FeatureAlgorithm * BA_itf );

		//new registration maps
	//typedef UNORDERED_MAP<string,Transform*> TransformMap;
	//TransformMap RegisteredTransforms;

	//typedef UNORDERED_MAP<string,FeatureAlgorithm*> AlgorithmMap;
	//AlgorithmMap RegisteredAlgorithms;

	//void dump_phonebook();

protected:
	FeatureNames();

private:
	static FeatureNames* pInstance;
	static bool instanceFlag;

////////////////////////////////////////
// Private static object caches
////////////////////////////////////////
	typedef UNORDERED_MAP<std::string, Channel *> cnm_t;
	cnm_t  channels_;

	typedef UNORDERED_MAP<std::string, Transform *> tnm_t;
	tnm_t  transforms_;

	typedef UNORDERED_MAP<std::string, FeatureAlgorithm *> fam_t;
	fam_t  feature_algorithms_;

	typedef UNORDERED_MAP<std::string, FeatureGroup *> fgnm_t;
	fgnm_t feature_groups_;

	typedef UNORDERED_MAP<std::string, FeatureInfo *> fnm_t;
	fnm_t  features_;

	typedef UNORDERED_MAP<std::string,std::string> ofnm_t;
	ofnm_t old_features_;


};

#endif // __FEATURE_NAMES_HPP__
