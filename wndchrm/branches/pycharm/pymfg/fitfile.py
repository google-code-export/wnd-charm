#!/usr/bin/env python

from pycharm import pymfg
import numpy as np
from StringIO import StringIO
from collections import defaultdict



def main():

	import pdb; pdb.set_trace()
	
	training_set = TrainingSet( '/Users/chris/projects/josiah_worms/terminal_bulb.fit' )

	# read in weights from a wndchrm html file
	# or just calculate them outright

	# now, give me only those columns that correspond with feature weights

	one_tile = training_set.sig_matrix[0,:-1]
	# do a classify operation

	fake_weights = [1] * training_set.num_features
	norm_factor, marg_probs = ClassifyWND5( training_set, one_tile, fake_weights )

	print "norm factor {}, marg probs {}".format( norm_factor, marg_probs )

#============================================================================
class TrainingSet:
	"""
  """
	num_classes = -1
	num_features = -1
	num_images = -1
	classnames_list = []
	featurenames_list = []
	tilenames_list = []
	sig_matrix = None

	def __init__( self, filename ):
		fitfile = open( filename )
		print "Creating Training Set from file {}".format( filename )

		data_lists = []

		name_line = False
		line_num = 0
		for line in fitfile:
			if line_num is 0:
				self.num_classes = int( line )
			elif line_num is 1:
				self.num_features = int( line )
			elif line_num is 2:
				self.num_images = int( line )
			elif line_num <= ( self.num_features + 2 ):
				self.featurenames_list.append( line.strip() )
			elif line_num == ( self.num_features + 3 ):
				pass # skip a line
			elif line_num <= ( self.num_features + 3 + self.num_classes ):
				self.classnames_list.append( line.strip() )
			else:
				# read in features
				if name_line:
					self.tilenames_list.append( line.strip() )
				else:
					data_lists.append( line.strip() )
				name_line = not name_line
			line_num += 1

		fitfile.close()

		string_data = "\n"
		
		self.sig_matrix = np.genfromtxt( StringIO( string_data.join( data_lists ) ) )
		# normalize the features at some point
		
		# class_ids should be zero-indexed
		if int( np.amin( self.sig_matrix[:, -1] ) ) == 1:
			self.sig_matrix = self.sig_matrix - 1

			

def ClassifyWND5( trainingset, testimg, feature_weights ):
	"""
	If you're using this function, your training set data is not continuous.
	For N images and M features:
		trainingset is of type TrainingSet with N x M+1 numpy matrix (+1 is ground truth)
		testtile is a 1 x M list of feature values
	NOTE: the trainingset and test image must have the same number of features!!!
	Returns a tuple with norm factor and list of length L of marginal probabilities
	FIXME: what about tiling??
	"""


	print "classifying..."
	epsilon = np.finfo( np.float ).eps
	#EpsTest = np.vectorize( lambda x: 0 if x < epsilon else x )

	num_features_in_testimg = len( testimg ) 
	weights_squared = np.square( feature_weights )

	# Create a view to the trainingset matrix that doesn't include the last ground truth column
	sig_matrix = trainingset.sig_matrix[:,:-1]
	class_ids = trainingset.sig_matrix[:,-1]

	class_similarities = [0.0] * trainingset.num_classes
	class_tile_counts = [0.0] * trainingset.num_classes

	num_tiles, num_features = sig_matrix.shape
	assert num_features_in_testimg == num_features,\
	"num features {}, num features in test img {}".format( num_features, num_test_img_features )

	#print "num tiles: {}, num_test_img_features {}".format( num_tiles, num_test_img_features )
	for tile_index in range( (num_tiles) ):
		#print "{} ".format( tile_index )
		
		class_index = int( class_ids[ tile_index ] )
		wnd_sum = 0
		dists = []
		dists = np.absolute( sig_matrix[ tile_index ] - testimg )
		# dists = EpsTest( np.absolute( sig_matrix[ tile_index ] - testimg ) )
		# epsilon checking for each feature is too expensive
		# do this quick and dirty check until we can figure something else out
		w_dist = np.sum( dists )
		if w_dist < epsilon:
			continue
		class_tile_counts[ class_index ] += 1
		w_dist = np.sum( np.multiply( weights_squared, np.square( dists ) ) )
		class_similarities[ class_index ] += w_dist ** -5

	for class_index in range( len( class_similarities ) ):
		class_similarities[ class_index ] /= class_tile_counts[ class_index ]

	normalization_factor = sum( class_similarities )

	return ( normalization_factor, [ x / normalization_factor for x in class_similarities ] ) 



#================================================================
if __name__=="__main__":
	main()


