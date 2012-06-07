#!/usr/bin/env python

import numpy as np
from os import path as path_parse
import itertools # for islice()
try:
	import cPickle as pickle
except:
	import pickle
from pycharm import pymfg
from pycharm import FeatureNameMap
import TrainingSets
from TrainingSets import TrainingSet
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
	offset = 0
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

	# LOAD FEATURE WEIGHTS
	use_these_feature_weights = None
	if not use_pickle:
		# load feature weights
		weights = TrainingSet.FeatureWeights.NewFromFile( weights_filepath )
		weights.Threshold( num_features_to_be_used )

		weights.names = FeatureNameMap.Translate( weights.names )

		with open( weights_filepath + "_n{}.pickled".format( num_features_to_be_used ), 'wb') as outfile:
			pickle.dump( use_these_feature_weights, outfile, pickle.HIGHEST_PROTOCOL )
	else:
		with open( weights_filepath + "_n{}.pickled".format( num_features_to_be_used ), "rb" ) as pkled_in:
			use_these_feature_weights = pickle.load( pkled_in )



	# LOAD TRAINING SET
	training_set = None
	pickle_path = classifier_filepath.rstrip( '.fit' )
	pickle_path += "_optimized_n{}.fit.pickled".format( num_features_to_be_used )

	if not use_pickle:
		# load training_set
		full_ts = TrainingSet.NewFromFitFile( classifier_filepath )
		full_ts.featurenames_list =\
				TrainingSets.TranslateFeatureNames( name_dict, full_ts.featurenames_list )
		training_set = full_ts.FeatureReduce( weight_names )
		training_set.Normalize()
		training_set.PickleMe( pickle_path )
	else:
		training_set = TrainingSet.NewFromPickleFile( pickle_path )


	# CALCULATE FEATURES
	work_order, num_output_features = \
			Signatures.GenerateWorkOrderFromListOfFeatureStrings( weight_names, algs, tforms )
	longer_nameslist = []

	signature = Signatures.Signatures.FromFeatureGroupList( inputimage_filepath, work_order )

	# WRITE CALCULATED FEATURES TO FILE
	input_path, input_filename = path_parse.split( inputimage_filepath )
	tmp = input_filename.rsplit( ".", 1 )
	outfile_name = tmp[0] + ".pysig"
	signature.WriteFeaturesToSigFile( 
			input_path+'/'+outfile_name, longer_nameslist, longer_featurevector )

	# LOAD CALCULATED FEATURES INTO A TEST SET
	test_set = TrainingSet.TrainingSet.NewFromSignature( signature )
	test_set = test_set.FeatureReduce( weight_names )
	training_set.Normalize( test_set )

	#CLASSIFY
	TrainingSets.ClassifyTestSet( training_set, test_set, weight_values )

if __name__=="__main__":
	main()
