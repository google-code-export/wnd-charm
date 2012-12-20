#!/usr/bin/env python
# -------- preamble to get the test data --------------------
from pychrm.TrainingSet import *
import os
import sys
# This script is in googlecode/pychrm/trunk/tests/
# test data in googlecode/wndchrm/tests
# get the directory three levels up from this script, then wndchrm/tests
my_dir = os.path.dirname(os.path.realpath(__file__))
if (len (sys.argv) > 1):
	test_dir = sys.argv[1]
	if not os.path.isdir (test_dir):
		print "The supplied path to the tests directory '{0}' is not a directory".format (test_dir)
		test_dir = None
else:
	test_dir = os.path.join (os.path.dirname(os.path.dirname(os.path.dirname(my_dir))),'wndchrm','tests')
	if not os.path.isdir (test_dir):
		print "The path to the tests directory relative to this script '{0}' is not a directory".format (test_dir)
		test_dir = None
if not (test_dir):
	print "The tests directory can be checked out from svn and the test re-run using the following commands:"
	print "svn checkout http://wnd-charm.googlecode.com/svn/wndchrm/tests tests"
	print "{0} tests".format (sys.argv[0])
	sys.exit(0)
from pychrm import __version__ as pychrm_version
def roundToSigFigs(num, n):
	from math import log10, ceil, pow, fabs
	if(num == 0):
		return 0;
	d = ceil(log10(fabs(num)));
	power = n - int(d);
	magnitude = pow(10, power);
	shifted = round(num*magnitude);
	return shifted/magnitude;

# -------- END preamble to get the test data --------------------

test_name = "Feature calculation"
max_diff_pass = 0.002
max_mean_pass = 0.00001
sig_file = os.path.join (test_dir,'t1_s01_c05_ij-l_precalculated.sig')
test_tif = os.path.join (test_dir,'t1_s01_c05_ij.tif')

test_sigs = Signatures.NewFromSigFile( sig_file, image_path = test_tif )
calc_sig_names = FeatureNameMap.TranslateToNewStyle( test_sigs.names )
calc_sigs = Signatures.NewFromFeatureNameList ( test_tif, calc_sig_names )
# calc_sig_names = []
# for i in range(20):
# 	calc_sig_names.append ('Fractal Features (Chebyshev (Wavelet ())) [{0}]'.format (i))
# for i in range(28):
# 	calc_sig_names.append ('Haralick Textures (Fourier ()) [{0}]'.format (i))
# for i in range(24):
# 	calc_sig_names.append ('Multiscale Histograms () [{0}]'.format (i))
# for i in range(5):
# 	calc_sig_names.append ('Pixel Intensity Statistics (Fourier ()) [{0}]'.format (i))
# for i in range(5):
# 	calc_sig_names.append ('Pixel Intensity Statistics () [{0}]'.format (i))
# for i in range(5):
# 	calc_sig_names.append ('Pixel Intensity Statistics (Chebyshev ()) [{0}]'.format (i))
# for i in range(5):
# 	calc_sig_names.append ('Pixel Intensity Statistics (Wavelet ()) [{0}]'.format (i))
# for i in range(5):
# 	calc_sig_names.append ('Pixel Intensity Statistics (Edge ()) [{0}]'.format (i))
# for i in range(12):
# 	calc_sig_names.append ('Radon Coefficients (Fourier ()) [{0}]'.format (i))
# for i in range(6):
# 	calc_sig_names.append ('Tamura Textures (Fourier ()) [{0}]'.format (i))
# for i in range(48):
# 	calc_sig_names.append ('Comb Moments (Fourier ()) [{0}]'.format (i))
# for i in range(20):
# 	calc_sig_names.append ('Fractal Features (Fourier ()) [{0}]'.format (i))
# for i in range(32):
# 	calc_sig_names.append ('Chebyshev Coefficients (Fourier ()) [{0}]'.format (i))
# 
# calc_sigs = Signatures.NewFromFeatureNameList ( test_tif, calc_sig_names )
test_sigs = test_sigs.FeatureReduce( calc_sig_names )

epsilon = 0.002
sig_figs = 6
max_diff = 0.
sum_diff = 0.
num_diffs = 0.

for idx in range (len(calc_sigs.names)):
	test_val = test_sigs.values[idx]
	calc_val = calc_sigs.values[idx]
	diff = abs(calc_val - test_val)
# 	diff = abs(roundToSigFigs (calc_val,sig_figs) - roundToSigFigs (test_val,sig_figs))
	sum_diff += diff
	num_diffs += 1.0
	if diff > max_diff:
		max_diff = diff
	if ( diff > epsilon):
		print "computed sig '{0}' ({1}) differs from sig in file ({2}) by {3}".format (
			calc_sigs.names [idx], calc_val, test_val, diff )

print "{0} comparissons, maximum diff = {1}, mean = {2}".format ( int (num_diffs), max_diff, sum_diff / num_diffs )
if (max_diff > max_diff_pass or (sum_diff / num_diffs) > max_mean_pass):
	print "pychrm {0} {1} test: {2}".format (pychrm_version, test_name, 'FAIL')
else:
	print "pychrm {0} {1} test: {2}".format (pychrm_version, test_name, 'PASS')
