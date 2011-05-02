#ifndef __FEATURE_NAMES_HPP__
#define __FEATURE_NAMES_HPP__

#include <string>
#include <vector>
#include <set>
#include "config.h"
#include "MAP.h"  // Defines the macro MAP which decides whether to #include
                  // <unordered_map>, <tr1/unordered_map>, or just plain ol <map>
//=====================================================================
/*! Channels
 *
 */
class Channel {
	public:
		std::string name;

		Channel (std::string &s) { name = s;}
		Channel (const char *s) { name = s;}

		void print_info() const;
};

//=====================================================================
/*!
 * Transform
 */
class Transform {
	public:
		std::string name;

		Transform (std::string &s) { name = s;}
		Transform (const char *s) { name = s;}

		void print_info() const;
};
typedef std::vector<Transform const *> TransformList;

//=====================================================================
/*!
 * Feature Algorithms
 */
class FeatureAlgorithm {
	public:
		std::string name;
		int n_features;

		FeatureAlgorithm () : name(""), n_features(1) { }
		FeatureAlgorithm (std::string &s,int i) { name = s; n_features = i;}
		FeatureAlgorithm (const char *s,int i) { name = s; n_features = i;}

		void print_info() const;
};

//=====================================================================
/*!
 * Feature Groups
 */
class FeatureGroup {
	public:

		std::string name;
		const FeatureAlgorithm *algorithm;
		const Channel *channel;
		TransformList transforms; // these are in order of application

		FeatureGroup () : algorithm(NULL), channel(NULL) {};
		FeatureGroup (std::string &s, const FeatureAlgorithm *f, const Channel *c, std::vector<Transform const *> t) {
			name = s; algorithm = f; channel = c; transforms = t;
		}

		void print_info() const;
};
//=====================================================================
/*!
 * Features
 */
class FeatureInfo {
	public:
		std::string name;
		const FeatureGroup *group;
		int index; // within group

		FeatureInfo () : group(NULL), index(-1) {};
		FeatureInfo (std::string &s, const FeatureGroup *g, int i) { name = s; group = g; index = i;}

		void print_info() const;
};

//=====================================================================
class FeatureNames {
public:

	~FeatureNames() {instanceFlag = false; delete pInstance;};

	static FeatureNames* getInstance();

	//! This just returns the string, should return a channel object by string lookup
	const Channel *getChannelByName (std::string &name);

	//! This just returns the string, should return a transform object by string lookup
	const Transform *getTransformByName (std::string &name);

	//! This returns an iterator to the algorithm map by string lookup
	const FeatureAlgorithm *getFeatureAlgorithmByName (std::string &name);

	//! This will store a new group if the name doesn't exist.
	//! The returned pointer is to an iterator into the static group map
	const FeatureGroup *getGroupByName (std::string &name);

	const FeatureInfo *getFeatureInfoByName (const char *featurename_in);

	//! Old-style feature name lookup
	const std::string *oldFeatureNameLookup (const char *oldFeatureName);

	// These have to be public and static in order to be called pre-main without an object.
	// This is done in FeatureNames.cpp in the global scope - outside of any functions/methods.
	const bool initFeatureAlgorithms();
	const bool initOldFeatureNameLookup();

protected:
	FeatureNames();
private:
	static FeatureNames* pInstance;
	static bool instanceFlag;

////////////////////////////////////////
// Private static object caches
////////////////////////////////////////
	typedef MAP<std::string, Channel *> cnm_t;
	cnm_t  channels_;

	typedef MAP<std::string, Transform *> tnm_t;
	tnm_t  transforms_;

	typedef MAP<std::string, FeatureAlgorithm *> fam_t;
	fam_t  feature_algorithms_;

	typedef MAP<std::string, FeatureGroup *> fgnm_t;
	fgnm_t feature_groups_;

	typedef MAP<std::string, FeatureInfo *> fnm_t;
	fnm_t  features_;

	typedef MAP<std::string,std::string> ofnm_t;
	ofnm_t old_features_;
};

#endif // __FEATURE_NAMES_HPP__
