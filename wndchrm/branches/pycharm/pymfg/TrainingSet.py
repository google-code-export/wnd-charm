#!/usr/bin/env python

from pycharm import pymfg

# FeatureRegistration.py is where the SWIG wrapped objects get put into a dict
# for use in signature calculation
from pycharm import FeatureRegistration 

# FeatureNameMap.py contains mapping from old style names to new style
# and the function TranslateFeatureNames()
from pycharm import FeatureNameMap

import numpy as np
from StringIO import StringIO
try:
	import cPickle as pickle
except:
	import pickle

# os module has function os.walk( ... )
import os
import os.path 
import re


# Initialize module level globals
Algorithms = None
Transforms = None
small_featureset_featuregroup_strings = None
large_featureset_featuregroup_strings = None
small_featureset_featuregroup_list = None
large_featureset_featuregroup_list = None

def initialize_module(): 
	# If you're going to calculate any signatures, you need this stuff
	# FIXME: Maybe rig something up using a load on demand?
	global Algorithms
	global Transforms
	global small_featureset_featuregroup_strings
	global large_featureset_featuregroup_strings
	global small_featureset_featuregroup_list
	global large_featureset_featuregroup_list

	Algorithms = FeatureRegistration.LoadFeatureAlgorithms()
	Transforms = FeatureRegistration.LoadFeatureTransforms()

	feature_lists = FeatureRegistration.LoadSmallAndLargeFeatureSetStringLists()

	small_featureset_featuregroup_strings = feature_lists[0]
	full_list = "\n"
	large_featureset_featuregroup_strings = full_list.join( feature_lists )

	small_featureset_featuregroup_list = []
	for fg_str in small_featureset_featuregroup_strings.splitlines():
		fg = ParseFeatureGroupString( fg_str )
		small_featureset_featuregroup_list.append( fg )

	large_featureset_featuregroup_list = []
	for fg_str in large_featureset_featuregroup_strings.splitlines():
		fg = ParseFeatureGroupString( fg_str )
		large_featureset_featuregroup_list.append( fg )



#############################################################################
# class definition of Signatures
#############################################################################
class Signatures:
	"""
	"""

	path_to_image_file = None
	feature_names = None
	signatures = None
	options = ""

	#================================================================
	def __init__( self ):
		"""@brief: constructor
		"""
		self.feature_names = []
		self.feature_values = []

	#================================================================
	@classmethod
	def SmallFeatureSet( cls, path ):
		"""@brief Equivalent of invoking wndchrm train in c-chrm
		@return An instance of the class Signatures for image with sigs calculated."""

		print "====================================================================="
		print "Calculating small feature set for file:"
		global small_featureset_featuregroup_list
		return cls.FromFeatureGroupList( path, small_featureset_featuregroup_list )

	#================================================================
	@classmethod
	def LargeFeatureSet( cls, path ):
		"""@brief Equivalent of invoking wndchrm train -l in c-chrm
		@return An instance of the class Signatures for image with sigs calculated."""

		print "====================================================================="
		print "Calculating large feature set for file:"
		global large_featureset_featuregroup_list
		return cls.FromFeatureGroupList( path, large_featureset_featuregroup_list, "-l" )

	#================================================================
	@classmethod
	def FromFeatureGroupList( cls, path, feature_groups, options = None ):
		"""@brief calculates signatures

		@remarks: Currently, you are not allowed to ask for a Signatures using a feature list,
		but instead use this call with a feature group list, load the signature into a
		TrainingSet instance, then call TrainingSet.FeatureReduce.
		"""
		print path
		original = pymfg.ImageMatrix()
		if 1 != original.OpenImage( path, 0, None, 0, 0 ):
			raise ValueError('Could not build an ImageMatrix from {}, check the path.'.format( path ))

		im_cache = {}
		im_cache[ '' ] = original

		# instantiate an empty Signatures object
		signatures = cls()
		signatures.path_to_image_file = path
		signatures.options = options

		for fg in feature_groups:
			print "Group {}".format( fg.Name )
			feature_vector = fg.CalculateFeatures( im_cache )
			count = 0
			for value in feature_vector:
				signatures.feature_names.append( fg.Name + " [{}]".format( count ) )
				signatures.feature_values.append( value )	
				count += 1

		return signatures

	#================================================================
	@classmethod
	def FromPickle( cls, path ):
		"""
		FIXME: Implement!
		"""
		pass

	#================================================================
	def PickleMe( self ):
		"""
		FIXME: Implement!
		"""
		pass
	
	#================================================================
	def	WriteFeaturesToASCIISigFile( self, filepath = None ):
		"""Write a sig file.
		
		If filepath is specified, you get to name it whatever you want and put it
		wherever you want. Otherwise, it's named according to convention and placed 
		next to the image file in its directory."""

		self.is_valid()

		outfile_path = ""
		if not filepath or filepath == "":
			if not self.path_to_image_file or self.path_to_image_file == "":
				raise ValueError( "Can't write sig file. No filepath specified in function call, and no path associated with this instance of Signatures." )
			outfile_path = self.path_to_image_file

			path, filename = os.path.split( outfile_path )
			if not os.path.exists( path ):
				raise ValueError( 'Invalid path {}'.format( path ) )

			filename_parts = filename.rsplit( '.', 1 )
			if self.options and self.options is not "":
				outfile_path = "{}{}.pysig".format( filename_parts[0],\
																					self.options if self.options else "" )
			else:
				outfile_path = "{}.pysig".format( filename_parts[0] )
			outfile_path = os.path.join( path, outfile_path )

		if os.path.exists( outfile_path ):
			print "Overwriting {}".format( outfile_path )
		else:
			print 'Writing signature file "{}"'.format( outfile_path )

		with open( outfile_path, "w" ) as out_file:
			# FIXME: line 2 contains class membership, just hardcode a number for now
			out_file.write( "0\n" )
			out_file.write( "{}\n".format( self.path_to_image_file ) )
			for i in range( 0, len( self.feature_names ) ):
				out_file.write( "{val:0.6f} {name}\n".format( val=self.feature_values[i], name=self.feature_names[i] ) )


	#================================================================
	def is_valid( self ):
		"""
		@brief: a signatures instance should know all the criteria for being a valid signature
		"""
		if len( self.feature_values ) <= 0:
			raise ValueError( 'Cannot add the signature to the training set, there are no feature values in it!' )
		assert len( self.feature_values ) == len( self.feature_names ),\
				"Can't add signature to training set, signature doesn't have the same number of values ({}) and names ({}).".format( len( signature.feature_values ), len( signature.feature_names ) )
		return True


# end definition class Signatures

#############################################################################
# class definition of FeatureGroup
#############################################################################
class FeatureGroup:
	"""
	Attributes Name, Alg and Tforms are references to the SWIG objects
	"""

	Name = ""
	Alg = None
	Tforms = []
	def __init__( self, name_str = "", algorithm = None, tform_list = [] ):
		#print "Creating new FeatureGroup for string {}:".format(name_str)
		#print "\talgorithm: {}, transform list: {}".format( algorithm, tform_list )
		self.Name = name_str 
		self.Alg = algorithm
		self.Tforms = tform_list
	def CalculateFeatures( self, cached_pixel_planes ):
		"""Returns a tuple with the features"""
		pixel_plane = None
		try:
			#print "transforms: {}".format( self.Tforms )
			pixel_plane = RetrievePixelPlane( cached_pixel_planes, self.Tforms )
		except:
			raise
		return self.Alg.calculate( pixel_plane )


#############################################################################
# global functions
#############################################################################
def RetrievePixelPlane( image_matrix_cache, tform_list ):
	"""
	Returns the image matrix prescribed in tform_list
	If it already exists in cache, just return.
	If it doesn't exist calculates it
	Recurses through the compound transform chain in tform_list
	"""
	#print "passed in: {}".format( tform_list )
	requested_transform = " ".join( [ tform.name for tform in tform_list ] )
	#print "requesting pixel plane: '{}'".format( requested_transform )
	if requested_transform in image_matrix_cache:
		return image_matrix_cache[ requested_transform ]
	
	# Required transform isn't in the cache, gotta make it
	# Pop transforms off the end sequentially and check to see if
	# lower-level transforms have already been calculated and stored in cache

	# Can't begin if there isn't at least the raw (untransformed) pixel plane
	# already stored in the cache
	if image_matrix_cache is None or len(image_matrix_cache) == 0:
		raise ValueError( "Can't calculate features: couldn't find the original pixel plane" +\
		                  "to calculate features {}.".format( self.Name ) )

	sublist = tform_list[:]
	sublist.reverse()
	top_level_transform = sublist.pop()
	intermediate_pixel_plane = RetrievePixelPlane( image_matrix_cache, sublist )

	tformed_pp = top_level_transform.transform( intermediate_pixel_plane )
	#assert( intermediate_pixel_plane ), "Pixel Plane returned from transform() was NULL"
	image_matrix_cache[ requested_transform ] = tformed_pp
	return tformed_pp


#================================================================
def ParseFeatureGroupString( name ):
	"""Takes a string input, parses, and returns an instance of a FeatureGroup class"""
	#TBD: make a member function of the FeatureGroup
	# get the algorithm

	global Algorithms
	global Transforms
	string_rep = name.rstrip( ")" )
	parsed = string_rep.split( ' (' )
	
	alg = parsed[0]
	if alg not in Algorithms:
		raise KeyError( "Don't know about a feature algorithm with the name {}".format(alg) )
	
	tform_list = parsed[1:]
	try:
		tform_list.remove( "" )
	except ValueError:
		pass
	if len(tform_list) != 0:
		for tform in tform_list:
			if tform not in Transforms:
				raise KeyError( "Don't know about a transform named {}".format( tform ) )

	tform_swig_obj_list = [ Transforms[ tform ] for tform in tform_list ]

	return FeatureGroup( name, Algorithms[ alg ], tform_swig_obj_list )

#================================================================
def GenerateWorkOrderFromListOfFeatureStrings( feature_list ):
	"""
	Takes list of feature strings and chops off bin number at the first space on right, e.g.,
	"feature alg (transform()) [bin]" ... Returns a list of FeatureGroups.
	@return work_order - list of FeatureGroup objects
	@return output_features_count - total number of individual features contained in work_order
	"""

	feature_group_strings = set()
	output_features_count = 0

	for feature in feature_list:
		split_line = feature.rsplit( " ", 1 )
		# add to set to ensure uniqueness
		feature_group_strings.add( split_line[0] )

	# iterate over set and construct feature groups
	work_order = []
	for feature_group in feature_group_strings:
		fg = ParseFeatureGroupString( feature_group )
		output_features_count += fg.Alg.n_features
		work_order.append( fg )

	return work_order, output_features_count

#############################################################################
# class definition of TrainingSet
#############################################################################
class TrainingSet:
	"""
  """

	# Source_file is essentially a name - might want to make separate name member in the future
	source_file = ""
	num_classes = -1
	num_features = -1
	num_images = -1

	# For C classes, each with Ni images and M features:
	# If the dataset is contiguous, C = 1

	# A list of numpy matrices, length C (one Ni x M matrix for each class)
	# The design is such because it's useful to be able to quickly collect feature statistics
	# across an image class excluding the other classes
	data_list = None

	# A list of strings, length C
	classnames_list = None

	# A list of strings length M
	featurenames_list = None

	# a list of lists, length C, where each list is length Ni, contining pathnames of tiles/imgs
	imagenames_list = None

	# The following class members are optional:
	# normalized_against is a string that keeps track of whether or not self has been
	# normalized. For test sets, value will be the source_file of the training_set.
	# For training sets, value will be "itself"
	normalized_against = None

	# Stored feature maxima and minima go in here
	# only exist, if self has been normalized against itself
	feature_maxima = None
	feature_minima = None

	# A list of floats against which marg probs can be multiplied
	# to obtain an interpolated value
	interpolation_coefficients = None

	# keep track of all the options (-l -S###, etc)
	# FIXME: expand to have all options kept track of individually
	feature_options = None

	def __init__( self, data_dict = None):
		"""
		TrainingSet constructor
		"""

		self.data_list = []
		self.classnames_list = []
		self.featurenames_list = []
		self.imagenames_list = []

		if data_dict != None:
			if "source_file" in data_dict:
				self.source_file = data_dict[ 'source_file' ]
			if "num_classes" in data_dict:
				self.num_classes = data_dict[ 'num_classes' ]
			if "num_features" in data_dict:
				self.num_features = data_dict[ 'num_features' ]
			if "num_images" in data_dict:
				self.num_images = data_dict[ 'num_images' ]
			if "data_list" in data_dict:
				self.data_list = data_dict[ 'data_list' ]
			if "classnames_list" in data_dict:
				self.classnames_list = data_dict[ 'classnames_list' ]
			if "featurenames_list" in data_dict:
				self.featurenames_list = data_dict[ 'featurenames_list' ]
			if "imagenames_list" in data_dict:
				self.imagenames_list = data_dict[ 'imagenames_list' ]
			if "feature_maxima" in data_dict:
				self.feature_maxima = data_dict[ 'feature_maxima' ]
			if "feature_minima" in data_dict:
				self.feature_minima = data_dict[ 'feature_minima' ]
			if "interpolation_coefficients" in data_dict:
				self.interpolation_coefficients = data_dict[ 'interpolation_coefficients' ]

  #=================================================================================
	@classmethod
	def NewFromPickleFile( cls, pathname ):
		"""
		The pickle is in the form of a dict
		FIXME: Shouldn't call Normalize if feature_maxima/minima are in the data_dict
		"""
		path, filename = os.path.split( pathname )
		if filename == "":
			raise ValueError( 'Invalid pathname: {}'.format( pathname ) )

		if not filename.endswith( ".fit.pickled" ):
			raise ValueError( 'Not a pickled TrainingSet file: {}'.format( pathname ) )

		print "Loading Training Set from pickled file {}".format( pathname )
		unpkled = None
		the_training_set = None
		with open( pathname, "rb" ) as pkled_in:
			the_training_set = cls( pickle.load( pkled_in ) )

		# it might already be normalized!
		# FIXME: check for that
		# the_training_set.Normalize()

		return the_training_set

  #=================================================================================
	@classmethod
	def NewFromFitFile( cls, pathname ):
		"""
		Helper function which reads in a c-chrm fit file, builds a dict with the info
		Then calls the constructor and passes the dict as an argument
		"""
		path, filename = os.path.split( pathname )
		if filename == "":
			raise ValueError( 'Invalid pathname: {}'.format( pathname ) )

		if not filename.endswith( ".fit" ):
			raise ValueError( 'Not a .fit file: {}'.format( pathname ) )

		pickled_pathname = pathname + ".pychrm"

		print "Creating Training Set from legacy WND-CHARM text file file {}".format( pathname )
		with open( pathname ) as fitfile:
			data_dict = {}
			data_dict[ 'source_file' ] = pathname
			data_dict[ 'data_list' ] = []
			data_dict[ 'imagenames_list' ] = []
			data_dict[ 'featurenames_list' ] = []
			data_dict[ 'classnames_list' ] = []
			data_dict[ 'imagenames_list' ] = []
			data_dict[ 'data_list' ] = []
			tmp_string_data_list = []

			name_line = False
			line_num = 0
			feature_count = 0
			image_pathname = ""
			num_classes = 0
			num_features = 0

			for line in fitfile:
				if line_num is 0:
					num_classes = int( line )
					data_dict[ 'num_classes' ] = num_classes
					# initialize list for string data
					for i in range( num_classes ):
						tmp_string_data_list.append( [] )
						data_dict[ 'imagenames_list' ].append( [] )
				elif line_num is 1:
					num_features = int( line )
					data_dict[ 'num_features' ] = num_features
				elif line_num is 2:
					data_dict[ 'num_images' ] = int( line )
				elif line_num <= ( num_features + 2 ):
					data_dict[ 'featurenames_list' ].append( line.strip() )
					feature_count += 1
				elif line_num == ( num_features + 3 ):
					pass # skip a line
				elif line_num <= ( num_features + 3 + num_classes ):
					data_dict[ 'classnames_list' ].append( line.strip() )
				else:
					# Read in features
					# Comes in alternating lines of data, then tile name
					if not name_line:
						# strip off the class identity value, which is the last in the array
						split_line = line.strip().rsplit( " ", 1)
						#print "class {}".format( split_line[1] )
						zero_indexed_class_id = int( split_line[1] ) - 1
						tmp_string_data_list[ zero_indexed_class_id ].append( split_line[0] )
					else:
						image_pathname = line.strip()
						data_dict[ 'imagenames_list' ][ zero_indexed_class_id ].append( image_pathname )
					name_line = not name_line
				line_num += 1

		string_data = "\n"
		
		for i in range( num_classes ):
			print "generating matrix for class {}".format( i )
			#print "{}".format( tmp_string_data_list[i] )
			npmatr = np.genfromtxt( StringIO( string_data.join( tmp_string_data_list[i] ) ) )
			data_dict[ 'data_list' ].append( npmatr )

		# Can the class names be interpolated?
		tmp_vals = []
		for class_index in range( num_classes ):
			m = re.search( r'(\d*\.?\d+)', data_dict[ 'classnames_list' ][class_index] )
			if m:
				tmp_vals.append( float( m.group(1) ) )
			else:
				tmp_vals = None
				break
		if tmp_vals:
			data_dict[ 'interpolation_coefficients' ] = tmp_vals

		# Instantiate the class
		the_training_set = cls( data_dict )

		# normalize the features
		#the_training_set.Normalize()
		# no wait, don't normalize until we feature reduce!
		
		return the_training_set

  #=================================================================================
	@classmethod
	def NewFromSignature( cls, signature, ts_name = "TestSet", ):
		"""@brief Creates a new TrainingSet from a single signature
		Was written with performing a real-time classification in mind.
		"""

		try:
			signature.is_valid()
		except:
			raise

		new_ts = cls()
		new_ts.source_file = ts_name
		new_ts.num_classes = 1
		new_ts.num_features = len( signature.feature_values )
		new_ts.num_images = 1
		new_ts.classnames_list.append( "UNKNOWN" )
		new_ts.featurenames_list = signature.feature_names
		new_ts.imagenames_list.append( [ inputimage_filepath ] )
		numpy_matrix = np.array( signature.feature_values )
		new_ts.data_list.append( numpy_matrix )

		return new_ts

  #=================================================================================
	@classmethod
	def NewFromDirectory( cls, top_level_dir_path, feature_set = "large", write_sig_files_todisk = True ):
		"""
		@brief A quick and dirty implementation of the wndchrm train command
		Build up the self.imagenames_list, then pass it off to a sig classifier function
		"""
		print "Creating Training Set from directories of images {}".format( top_level_dir_path )
		if not( os.path.exists( top_level_dir_path ) ):
			raise ValueError( 'Path "{}" doesn\'t exist'.format( top_level_dir_path ) )
		if not( os.path.isdir( top_level_dir_path ) ):
			raise ValueError( 'Path "{}" is not a directory'.format( top_level_dir_path ) )

		num_images = 0
		num_classes = 0
		classnames_list = []
		imagenames_list = []

		for root, dirs, files in os.walk( top_level_dir_path ):
			if root == top_level_dir_path:
				if len( dirs ) <= 0:
					# no class structure
					file_list = []
					for file in files:
						if '.tif' in file:
							file_list.append( os.path.join( root, file ) )
					if len( file_list ) <= 0:
						# nothing here to process!
						raise ValueError( 'No tiff files in directory {}'.format( root ) )
					classnames_list.append( root )
					num_classes = 1
					num_images = len( file_list )
					imagenames_list.append( file_list )
					break
			else:
				file_list = []
				for file in files:
					if '.tif' in file:
						file_list.append( os.path.join( root, file ) )
				if len( file_list ) <= 0:
					# nothing here to process!
					continue
				num_images += len( file_list )
				num_classes += 1
				imagenames_list.append( file_list )

		if num_classes <= 0:
			raise ValueError( 'No valid images or directories of images in this directory' )

		# instantiate a new training set
		new_ts = cls()
		new_ts.num_images = num_images
		new_ts.num_classes = num_classes
		new_ts.classnames_list = classnames_list
		new_ts.imagenames_list = imagenames_list
		new_ts._ProcessSigCalculationSerially( feature_set, write_sig_files_todisk )
		return new_ts


  #=================================================================================
	@classmethod
	def NewFromFileOfFiles( cls, fof_path, feature_set = "large", write_sig_files_todisk = True ):
		"""FIXME: Implement!"""
		pass

  #=================================================================================
	@classmethod
	def NewFromSQLiteFile(cls, path):
		"""FIXME: Implement!"""
		pass


  #=================================================================================
	def _ProcessSigCalculationSerially( self, feature_set = "large", write_sig_files_to_disk = True ):
		"""
		Work off the self.imagenames_list
		"""

		sig = None
		class_id = 0
		for class_filelist in self.imagenames_list:
			for file in class_filelist:
				if feature_set == "large":
					sig = Signatures.LargeFeatureSet( file )
				elif feature_set == "small":
					sig = Signatures.SmallFeatureSet( file )
				else:
					raise ValueError( "sig calculation other than small and large feature set hasn't been implemented yet." )
				# FIXME: add all the other options
				if write_sig_files_to_disk:
					sig.WriteFeaturesToASCIISigFile()
				self.AddSignature( sig, class_id )
			class_id += 1


  #=================================================================================
	def _ProcessSigCalculationParallelly( self, feature_set = "large", write_sig_files_todisk = True ):
		"""
		FIXME: When we figure out concurrency
		"""
		pass


  #=================================================================================
	def Normalize( self, test_set = None ):
		"""
		By convention, the range of values are normalized on an interval [0,100]
		FIXME: edge cases, clipping, etc
		"""

		if not( self.normalized_against ):

			full_stack = np.vstack( self.data_list )
			total_num_imgs, num_features = full_stack.shape
			self.feature_maxima = [None] * num_features
			self.feature_minima = [None] * num_features

			for i in range( num_features ):
				feature_max = np.max( full_stack[:,i] )
				self.feature_maxima[ i ] = feature_max
				feature_min = np.min( full_stack[:,i] )
				self.feature_minima[ i ] = feature_min
				for class_matrix in self.data_list:
					class_matrix[:,i] -= feature_min
					class_matrix[:,i] /= (feature_max - feature_min)
					class_matrix[:,i] *= 100
			self.normalized_against = "itself"

		if test_set:

			# sanity checks
			if test_set.normalized_against:
				raise ValueError( "Test set {} has already been normalized against {}."\
						.format( test_set.source_file, test_set.normalized_against ) )
			if test_set.featurenames_list != self.featurenames_list:
				raise ValueError("Can't normalize test_set {} against training_set {}: Features don't match."\
						.format( test_set.source_file, self.source_file ) )

			for i in range( test_set.num_features ):
				for class_matrix in test_set.data_list:
					class_matrix[:,i] -= self.feature_minima[i]
					class_matrix[:,i] /= (self.feature_maxima[i] - self.feature_minima[i])
					class_matrix[:,i] *= 100

			test_set.normalized_against = self.source_file
			

  #=================================================================================
	def FeatureReduce( self, requested_features ):
		"""
		Returns a new TrainingSet that contains a subset of the features
		arg requested_features is a tuple of features
		the returned TrainingSet will have features in the same order as they appear in
		     requested_features
		"""

		# Check that self's faturelist contains all the features in requested_features

		selfs_features = set( self.featurenames_list )
		their_features = set( requested_features )
		if not their_features <= selfs_features:
			raise ValueError( 'ERROR: Not all the features you asked for are in this training set.\n'+\
					'The following features are missing: {}'.format( their_features - selfs_features ) )

		# copy everything but the signature data
		reduced_ts = TrainingSet()
		reduced_ts.source_file = self.source_file + "(feature reduced)"
		reduced_ts.num_classes = self.num_classes
		assert reduced_ts.num_classes == len( self.data_list )
		new_num_features = len( requested_features )
		reduced_ts.num_features = new_num_features
		reduced_ts.num_images = self.num_images
		reduced_ts.imagenames_list = self.imagenames_list[:] # [:] = deepcopy
		reduced_ts.classnames_list = self.classnames_list[:]
		reduced_ts.featurenames_list = requested_features[:]
		reduced_ts.interpolation_coefficients = self.interpolation_coefficients[:]
		reduced_ts.feature_maxima = [None] * new_num_features
		reduced_ts.feature_minima = [None] * new_num_features

		# copy feature minima/maxima
		if self.feature_maxima and self.feature_minima:
			new_index = 0
			for featurename in requested_features:
				old_index = self.featurenames_list.index( featurename )
				reduced_ts.feature_maxima[ new_index ] = self.feature_maxima[ old_index ]
				reduced_ts.feature_minima[ new_index ] = self.feature_minima[ old_index ]
				new_index += 1

		# feature reduce
		for fat_matrix in self.data_list:
			num_imgs_in_class, num_old_features = fat_matrix.shape
			# NB: double parentheses required when calling numpy.zeros(), i guess it's a tuple thing
			new_matrix = np.zeros( ( num_imgs_in_class, new_num_features ) )
			new_column_index = 0
			for featurename in requested_features:
				fat_column_index = self.featurenames_list.index( featurename )
				new_matrix[:,new_column_index] = fat_matrix[:,fat_column_index]
				new_column_index += 1
			reduced_ts.data_list.append( new_matrix )

		return reduced_ts

  #=================================================================================
	def AddSignature( self, signature, class_id_index = None ):
		"""
		FIXME: implement!
		@argument class_id_index identifies the class to which the signature belongs
		"""
		
		try:
			signature.is_valid()
		except:
			raise

		if (self.data_list == None) or ( len( self.data_list ) == 0 ) :
			self.data_list = []
			self.featurenames_list = signature.feature_names
		else:
			if not( self.featurenames_list == signature.feature_names ):
				raise ValueError("Can't add the signature '{}' to training set because it contains different features.".format( signature.path_to_image_file ) )

		# signatures may be coming in out of class order
		while (len( self.data_list ) - 1) < class_id_index:
			self.data_list.append( None )

		if self.data_list[ class_id_index ] == None:
			self.data_list[ class_id_index ] = np.array( signature.feature_values )
		else:
			# vstack takes only one argument, a tuple, thus the extra set of parens
			self.data_list[ class_id_index ] = np.vstack( ( self.data_list[ class_id_index ] ,\
					np.array( signature.feature_values ) ) )


  #=================================================================================
	def CalculateFisherScores( self ):
		"""
		FIXME: implement!
		"""
		pass

  #=================================================================================
	def PickleMe( self, pathname ):
		"""
		FIXME: pathname needs to end with suffix '.fit.pickled'
		       or TrainingSet.FromPickleFile() won't read it.
		"""
		if os.path.exists( pathname ):
			print "Overwriting {}".format( pathname )
		with open( pathname, 'wb') as outfile:
			pickle.dump( self.__dict__, outfile, pickle.HIGHEST_PROTOCOL )


	def DumpNumpyArrays():
		pass
# END TrainingSet class definition


######################################################################################
# GLOBAL FUNCTIONS
######################################################################################

def WeightedNeighborDistance5( trainingset, testimg, feature_weights ):
	"""
	If you're using this function, your training set data is not continuous
	for N images and M features:
	  trainingset is list of length L of N x M numpy matrices
	  testtile is a 1 x M list of feature values
	NOTE: the trainingset and test image must have the same number of features!!!
	AND: the features must be in the same order!!
	Returns a tuple with norm factor and list of length L of marginal probabilities
	FIXME: what about tiling??
	"""

	#print "classifying..."
	epsilon = np.finfo( np.float ).eps

	num_features_in_testimg = len( testimg ) 
	weights_squared = np.square( feature_weights )

	# initialize
	class_similarities = [0] * trainingset.num_classes

	for class_index in range( trainingset.num_classes ):
		#print "Calculating distances to class {}".format( class_index )
		num_tiles, num_features = trainingset.data_list[ class_index ].shape
		assert num_features_in_testimg == num_features,\
		"num features {}, num features in test img {}".format( num_features, num_test_img_features )

		# create a view
		sig_matrix = trainingset.data_list[ class_index ]
		wnd_sum = 0
		num_collisions = 0

		#print "num tiles: {}, num_test_img_features {}".format( num_tiles, num_test_img_features )
		for tile_index in range( num_tiles ):
			#print "{} ".format( tile_index )
			# epsilon checking for each feature is too expensive
			# do this quick and dirty check until we can figure something else out
			dists = np.absolute( sig_matrix[ tile_index ] - testimg )
			w_dist = np.sum( dists )
			if w_dist < epsilon:
				num_collisions += 1
				continue
			dists = np.multiply( weights_squared, np.square( dists ) )
			w_dist = np.sum( dists )
			# The exponent -5 is the "5" in "WND5"
			class_similarities[ class_index ] += w_dist ** -5
		#print "\n"

		class_similarities[ class_index ] /= ( num_tiles - num_collisions )

	normalization_factor = sum( class_similarities )

	return ( normalization_factor, [ x / normalization_factor for x in class_similarities ] ) 

#=================================================================================
def ClassifyTestSet( training_set, test_set, feature_weights ):
	"""
	FIXME: What happens when the ground truth is not known? Currently they would all be shoved
	       into class 1, might not be a big deal since class name should be something
	       like "UNKNOWN"
	FIXME: return some python construct that contains classification results
	"""

	column_header = "image\tnorm. fact.\t"
	column_header +=\
			"".join( [ "p(" + class_name + ")\t" for class_name in training_set.classnames_list ] )

	column_header += "act. class\tpred. class\tpred. val."
	print column_header

	interp_coeffs = None
	if training_set.interpolation_coefficients:
		interp_coeffs = np.array( training_set.interpolation_coefficients )

	for test_class_index in range( test_set.num_classes ):
		num_class_imgs, num_class_features = test_set.data_list[ test_class_index ].shape
		for test_image_index in range( num_class_imgs ):
			one_image_features = test_set.data_list[ test_class_index ][ test_image_index,: ]
			normalization_factor, marginal_probabilities = \
					WeightedNeighborDistance5( training_set, one_image_features, feature_weights )

			# FIXME: call PrintClassificationResultsToSTDOUT( results )
			# img name:
			output_str = test_set.imagenames_list[ test_class_index ][ test_image_index ]
			# normalization factor:
			output_str += "\t{val:0.3g}\t".format( val=normalization_factor )
			# marginal probabilities:
			output_str += "".join(\
					[ "{val:0.3f}".format( val=prob ) + "\t" for prob in marginal_probabilities ] )
			output_str += test_set.classnames_list[ test_class_index ] + "\t"
			# actual class:
			output_str += test_set.classnames_list[ test_class_index ] + "\t"
			# predicted class:
			marg_probs = np.array( marginal_probabilities )
			output_str += "{}\t".format( training_set.classnames_list[ marg_probs.argmax() ] )
			# interpolated value, if applicable
			if interp_coeffs is not None:
				interp_val = np.sum( marg_probs * interp_coeffs )
				output_str += "{val:0.3f}".format( val=interp_val )
			print output_str

#=================================================================================
def PrintClassificationResultsToSTDOUT( result ):
	"""
	FIXME: Implement!
	"""
	pass

#=================================================================================
def ReadFeatureWeightsFromFile( weights_filepath ):
	
	feature_names = []
	feature_values = []
	with open( weights_filepath, 'r' ) as weights_file:
		for line in weights_file:
			# split line "number <space> name"
			feature_line = line.strip().split( " ", 1 )
			feature_values.append( float( feature_line[0] ) )
			feature_names.append( feature_line[1] )
	return feature_names, feature_values

#============================================================================
def UnitTest1():
	name_dict = FeatureNameMap.LoadFeatureNameTranslationDict()
	
	fitfilepath = '/Users/chris/projects/josiah_worms/terminal_bulb.fit'

	# read in weights from a c-chrm feature weights file
	weight_names, weight_values = ReadFeatureWeightsFromFile('/Users/chris/projects/josiah_worms/feature_weights.txt')
	# FIXME: just calculate them outright
	weight_names = FeatureNameMap.TranslateFeatureNames( name_dict, weight_names )
	fisher_scores = zip( weight_names, weight_values )
	nonzero_fisher_scores = [ (name, weight) for name, weight in fisher_scores if weight != 0 ]
	#print "{}".format( nonzero_weights )
	nonzero_fisher_names, nonzero_fisher_scores = zip( *nonzero_fisher_scores )

	reduced_ts = None

	if False:
		full_ts = TrainingSet.FromFitFile( fitfilepath )
		full_ts.featurenames_list = FeatureNameMap.TranslateFeatureNames( name_dict, full_ts.featurenames_list )
		reduced_ts = full_ts.FeatureReduce( nonzero_fisher_names )
		reduced_ts.Normalize()
		reduced_ts.PickleMe( '/Users/chris/projects/josiah_worms/terminal_bulb.fit.pickled' )
	else:
		reduced_ts = TrainingSet.FromPickleFile( '/Users/chris/projects/josiah_worms/terminal_bulb.fit.pickled' )

	#print "the lists are {} the same!".\
	#		format( "IN FACT" if reduced_ts.featurenames_list == nonzero_fisher_names else "NOT" )

	# classify 1 image
	#one_image = reduced_ts.data_list[0][0,:]
	#one_image_name = reduced_ts.imagenames_list[0][0]
	#norm_factor, marg_probs = WeightedNeighborDistance5( reduced_ts, one_image, nonzero_fisher_scores )
	#print "image {}, norm factor {}, marg probs {}".\
	#		format( one_image_name, norm_factor, marg_probs )

	# classify training set against itself
	ClassifyTestSet( reduced_ts, reduced_ts, nonzero_fisher_scores)


#=========================================================================
def UnitTest2():

	ts = TrainingSet.NewFromDirectory( '/Users/chris/projects/josiah_worms_subset',\
	                                   feature_set = "large" )
	ts.PickleMe()
	

#================================================================
def UnitTest3():

	path = "Y24-2-2_GREEN.tif"
	sigs = Signatures.LargeFeatureSet( path )
	sigs.WriteFeaturesToASCIISigFile( "pychrm_calculated.sig" )


initialize_module()

#================================================================
if __name__=="__main__":
	
	# UnitTest1()
	UnitTest2()
	# UnitTest3()
	# pass


