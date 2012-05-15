#!/usr/bin/env python

import numpy as np
from os import path as path_parse
import itertools # for islice()
try:
	import cPickle as pickle
except:
	import pickle
from pycharm import pymfg
import TrainingSets
import Signatures

# do this later
#import argparse
# usage: this image, against this classifier, with these weights,
#        using only N number of features

# FIXME: TrainingSets and Signatures need to be within pychrm namespace/module
def main():
	import time
	splits = []
	splits.append( 0.0 )
	offset = 106
	for i in range( offset, offset + 50 ):
		run_simulation( i )
		splits.append( time.time() )
	
	for i in range( 1, len( splits ) ):
		print "{}\t{}".format( offset+i-1, splits[i] - splits[i-1] )

def run_simulation( num_features_to_be_used ): 
	use_pickle = False

	inputimage_filepath = '/Users/chris/projects/josiah_worms/00_day/day0k.w3.tiff'
	weights_filepath = '/Users/chris/projects/josiah_worms/feature_weights.txt'
	classifier_filepath = '/Users/chris/projects/josiah_worms/terminal_bulb.fit'

	# initialize stuff
	algs = Signatures.LoadFeatureAlgorithms()
	tforms = Signatures.LoadFeatureTransforms()
	# open image and initialize pixel plane cache
	original = pymfg.ImageMatrix()
	if 1 != original.OpenImage( inputimage_filepath, 0, None, 0, 0 ):
		import sys; sys.exit()
	im_cache = {}
	im_cache[ '' ] = original


	# LOAD FEATURE WEIGHTS
	use_these_feature_weights = None
	if not use_pickle:
		# load feature weights
		weight_names, weight_values = TrainingSets.ReadFeatureWeightsFromFile( weights_filepath )
		name_dict = TrainingSets.LoadFeatureNameTranslationDict()
		weight_names = TrainingSets.TranslateFeatureNames( name_dict, weight_names )

		raw_featureweights = zip( weight_names, weight_values )
		# raw_featureweights is now a list of tuples := [ (name1, value1), (name2, value2), ... ]

		# sort from max to min
		# sort by the second item in the tuple, i.e., index 1
		sort_func = lambda feat_a, feat_b: cmp( feat_a[1], feat_b[1]) 
		#import pdb; pdb.set_trace()

		sorted_featureweights = sorted( raw_featureweights, sort_func, reverse = True )
		# take top N features
		use_these_feature_weights = \
				list( itertools.islice( sorted_featureweights, num_features_to_be_used ) )
		with open( weights_filepath + "_n{}.pickled".format( num_features_to_be_used ), 'wb') as outfile:
			pickle.dump( use_these_feature_weights, outfile, pickle.HIGHEST_PROTOCOL )
	else:
		with open( weights_filepath + "_n{}.pickled".format( num_features_to_be_used ), "rb" ) as pkled_in:
			use_these_feature_weights = pickle.load( pkled_in )

	weight_names, weight_values = zip( *use_these_feature_weights )


	# LOAD TRAINING SET
	training_set = None
	pickle_path = classifier_filepath.rstrip( '.fit' )
	pickle_path += "_optimized_n{}.fit.pickled".format( num_features_to_be_used )

	if not use_pickle:
		# load training_set
		full_ts = TrainingSets.TrainingSet.FromFitFile( classifier_filepath )
		full_ts.featurenames_list =\
				TrainingSets.TranslateFeatureNames( name_dict, full_ts.featurenames_list )
		training_set = full_ts.FeatureReduce( weight_names )
		training_set.Normalize()
		training_set.PickleMe( pickle_path )
	else:
		training_set = TrainingSets.TrainingSet.FromPickleFile( pickle_path )

	# CALCULATE FEATURES
	work_order, num_output_features = \
			Signatures.GenerateWorkOrderFromListOfFeatureStrings( weight_names, algs, tforms )
	longer_nameslist = []

	# preallocate array with len = num_features array
	# N.B. array shape must be enclosed in parentheses
	longer_featurevector = np.zeros( num_output_features )
	list_pos = 0
	#import pdb; pdb.set_trace()

	#feature_count = 0
	for feature_group in work_order:
		print "Group {}".format( feature_group.Name )
		#print "list position: {}".format( list_pos )
		returned_feature_vector = feature_group.CalculateFeatures( im_cache )
		n_features = len( returned_feature_vector )
		count = 0
		for value in returned_feature_vector:
			f_name = feature_group.Name + " [{}]".format( count )
			#print "FEATURE {}: {}".format( feature_count, f_name )
			longer_nameslist.append( f_name )
			#feature_count += 1
			count += 1
		# "broadcast" feature values into preallocated numpy array
		longer_featurevector[ list_pos:list_pos + n_features ] = returned_feature_vector
		list_pos += n_features

	# WRITE CALCULATED FEATURES TO FILE
	input_path, input_filename = path_parse.split( inputimage_filepath )
	tmp = input_filename.rsplit( ".", 1 )
	outfile_name = tmp[0] + ".pysig"
	Signatures.WriteFeaturesToSigFile( 
			input_path+'/'+outfile_name, longer_nameslist, longer_featurevector )

	# LOAD CALCULATED FEATURES INTO A TEST SET
	test_set = TrainingSets.TrainingSet()
	test_set.source_file = "TestSet"
	test_set.num_classes = 1
	test_set.num_features = num_output_features
	test_set.num_images = 1
	test_set.data_list.append( longer_featurevector )
	test_set.classnames_list.append( "UNKNOWN" )
	test_set.featurenames_list = longer_nameslist
	test_set.imagenames_list.append( [ inputimage_filepath ] )

	test_set = test_set.FeatureReduce( weight_names )
	training_set.Normalize( test_set )

	#CLASSIFY
	TrainingSets.ClassifyTestSet( training_set, test_set, weight_values )

if __name__=="__main__":
	main()
