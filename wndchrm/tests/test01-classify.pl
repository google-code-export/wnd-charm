#!/usr/bin/perl -w
# Tests classification - exact sample order, exact marginal probabilities, scale factors and interpolated values.
# Also tests exact final classification accuracy.
use strict;
use warnings;
use FindBin;
use lib $FindBin::Bin;
use TestUtil;

TestUtil::exit_fail("Please specify an executable\n") unless $ARGV[0];

my $ex = $ARGV[0];
my $path = TestUtil::getTestPath();

my @res = `$ex classify -l $path/test-l.fit $path/test-l.fit`;



my @lines = getExpectedClassifications();
my $expected_classifications = parseClassifications (\@lines);
my $reported_classifications = parseClassifications (\@res);
my $compRes;
if (! ($compRes = compareClassifications($reported_classifications,$expected_classifications)) ) {
	print "Exact comparison (order, scale, probs., values) for ".scalar(@$expected_classifications)." classifications - passed\n";
} else {
	print @res;
	TestUtil::exit_fail("Exact comparison failed: $compRes\n");
}


my $accuracy;
my $expected = 0.966667;
foreach (@res) {
	$accuracy = $1 if $_ =~ /^Accuracy:\s+(.+)$/;
}
TestUtil::exit_fail("FAILED - Accuracy not reported\n")
	unless $accuracy and TestUtil::is_numeric ($accuracy);

if (abs ($expected - $accuracy) < TestUtil::FLT_EPSILON) {
	TestUtil::exit_pass("passed - accuracy = $accuracy, expected = $expected\n");
} else {
	print @res;
	TestUtil::exit_fail("FAILED - accuracy = $accuracy, expected = $expected\n");
}

sub parseClassifications {
	my $lines = shift;
	my @classifications;
	foreach my $line (@$lines) {
		push (@classifications, [ split (/\s+/,$line) ]) if $line =~ /^[24]cell\/t\d+_s\d+_c\d+_ij.tif/;
	}
	return (\@classifications);
}

sub printClassifications {
	my $classifications = shift;

	foreach my $classification (@$classifications) {
		printf "Image [%s], scale: %g, p1: %g, p2: %g, act: [%s], pred: [%s], val: %g\n",
			$classification->[0], $classification->[1], $classification->[2], $classification->[3], 
			$classification->[4], $classification->[5], $classification->[6];
	}
}

sub compareClassifications {
	my ($reported_classifications,$expected_classifications) = (shift,shift);
	my $indx=0;
	
	for ($indx = 0; $indx < scalar(@$reported_classifications); $indx++) {
		my $expected = $expected_classifications->[$indx];
		my $reported = $reported_classifications->[$indx];
		
		return ("Image names don't match: Expected [".$expected->[0]."], reported: [".$reported->[0]."]" )
			unless $reported->[0] eq $expected->[0];
		return ( "Scale factors don't match for image [".$reported->[0]."]: Expected (".$expected->[1]."), reported: (".$reported->[1].")" )
			unless (abs ($reported->[1] - $expected->[1]) < TestUtil::FLT_EPSILON);
		return ( "Marg. probs. don't match for image [".$reported->[0]."]: Expected (".$expected->[2].",".$expected->[3]."), reported: (".$reported->[2].",".$reported->[3].")" )
			unless (abs ($reported->[2] - $expected->[2]) < TestUtil::FLT_EPSILON && abs ($reported->[3] - $expected->[3]) < TestUtil::FLT_EPSILON);
		return ( "Interpolated values don't match for image [".$reported->[0]."]: Expected [".$expected->[6]."], reported: [".$reported->[6]."]" )
			unless (abs ($reported->[6] - $expected->[6]) < TestUtil::FLT_EPSILON);
	}
	return ("");
}


sub getExpectedClassifications  {
my $expected_out = <<END;
2cell/t28_s02_c10_ij.tif	1.46e-21	0.952	0.048	2cell	2cell	2.097
2cell/t29_s03_c03_ij.tif	1.47e-22	0.835	0.165	2cell	2cell	2.330
2cell/t29_s09_c02_ij.tif	1.49e-21	0.978	0.022	2cell	2cell	2.043
2cell/t32_s04_c10_ij.tif	1.91e-22	0.939	0.061	2cell	2cell	2.123
2cell/t33_s09_c01_ij.tif	8.17e-22	0.871	0.129	2cell	2cell	2.257
2cell/t33_s09_c08_ij.tif	3.86e-22	0.947	0.053	2cell	2cell	2.106
2cell/t34_s03_c12_ij.tif	1.69e-22	0.823	0.177	2cell	2cell	2.354
2cell/t35_s04_c01_ij.tif	1.49e-22	0.913	0.087	2cell	2cell	2.175
2cell/t36_s03_c11_ij.tif	5.17e-22	0.931	0.069	2cell	2cell	2.137
2cell/t36_s10_c08_ij.tif	1.34e-22	0.895	0.105	2cell	2cell	2.210
2cell/t38_s02_c04_ij.tif	1.46e-22	0.958	0.042	2cell	2cell	2.084
2cell/t40_s04_c04_ij.tif	2.32e-22	0.662	0.338	2cell	2cell	2.676
2cell/t40_s07_c08_ij.tif	2.56e-22	0.604	0.396	2cell	2cell	2.791
2cell/t40_s10_c12_ij.tif	1.22e-21	0.911	0.089	2cell	2cell	2.177
2cell/t40_s9_c09_ij.tif	2.84e-22	0.912	0.088	2cell	2cell	2.175
2cell/t43_s03_c03_ij.tif	3.65e-22	0.935	0.065	2cell	2cell	2.130
2cell/t43_s03_c12_ij.tif	4.41e-22	0.891	0.109	2cell	2cell	2.217
2cell/t45_s01_c10_ij.tif	7.96e-22	0.844	0.156	2cell	2cell	2.312
2cell/t45_s06_c03_ij.tif	3.58e-22	0.990	0.010	2cell	2cell	2.020
2cell/t45_s06_c06_ij.tif	1.21e-22	0.294	0.706	2cell	4cell	3.412
2cell/t45_s06_c12_ij.tif	6.04e-22	0.932	0.068	2cell	2cell	2.136
2cell/t45_s07_c05_ij.tif	1.12e-21	0.956	0.044	2cell	2cell	2.087
2cell/t45_s07_c11_ij.tif	2.14e-22	0.981	0.019	2cell	2cell	2.038
2cell/t45_s11_c10_ij.tif	3.19e-22	0.456	0.544	2cell	4cell	3.088
2cell/t45_s13_c04_ij.tif	1.59e-22	0.717	0.283	2cell	2cell	2.566
2cell/t47_s08_c07_ij.tif	1.19e-22	0.942	0.058	2cell	2cell	2.116
2cell/t47_s08_c08_ij.tif	1.31e-22	0.948	0.052	2cell	2cell	2.104
2cell/t47_s8_c04_ij.tif	1.8e-21	0.959	0.041	2cell	2cell	2.081
2cell/t48_s11_c03_ij.tif	2.69e-22	0.926	0.074	2cell	2cell	2.149
2cell/t50_s04_c05_ij.tif	1.88e-22	0.910	0.090	2cell	2cell	2.179
2cell/t50_s05_c11_ij.tif	3.79e-22	0.960	0.040	2cell	2cell	2.081
2cell/t50_s10_c05_ij.tif	6.29e-22	0.977	0.023	2cell	2cell	2.047
2cell/t52_s3_c10_ij.tif	4.38e-22	0.847	0.153	2cell	2cell	2.306
2cell/t52_s9_c10_ij.tif	5.39e-22	0.964	0.036	2cell	2cell	2.072
2cell/t54_s12_c03_ij.tif	4.69e-22	0.925	0.075	2cell	2cell	2.150
2cell/t54_s12_c04_ij.tif	1.57e-22	0.808	0.192	2cell	2cell	2.384
2cell/t54_s12_c06_ij.tif	1.61e-22	0.968	0.032	2cell	2cell	2.064
2cell/t54_s12_c08_ij.tif	5.58e-22	0.973	0.027	2cell	2cell	2.054
2cell/t54_s12_c11_ij.tif	3.59e-22	0.915	0.085	2cell	2cell	2.169
2cell/t55_s03_c03_ij.tif	1.03e-21	0.943	0.057	2cell	2cell	2.114
2cell/t55_s05_c09_ij.tif	6.28e-22	0.936	0.064	2cell	2cell	2.129
2cell/t55_s05_c12_ij.tif	1.11e-22	0.654	0.346	2cell	2cell	2.692
2cell/t55_s10_c11_ij.tif	1.73e-22	0.950	0.050	2cell	2cell	2.099
2cell/t60_s02_c10_ij.tif	1.36e-23	0.438	0.562	2cell	4cell	3.123
2cell/t60_s02_c11_ij.tif	2.5e-22	0.935	0.065	2cell	2cell	2.129
4cell/t103_s12_c11_ij.tif	6.83e-22	0.152	0.848	4cell	4cell	3.697
4cell/t106_s01_c09_ij.tif	6.17e-22	0.043	0.957	4cell	4cell	3.914
4cell/t108_s03_c10_ij.tif	1.19e-21	0.010	0.990	4cell	4cell	3.980
4cell/t108_s06_c04_ij.tif	4.52e-22	0.189	0.811	4cell	4cell	3.622
4cell/t108_s06_c09_ij.tif	6.73e-22	0.082	0.918	4cell	4cell	3.836
4cell/t109_s11_c09_ij.tif	1.38e-22	0.006	0.994	4cell	4cell	3.989
4cell/t110_s03_c12_ij.tif	2.59e-22	0.471	0.529	4cell	4cell	3.058
4cell/t110_s10_c12_ij.tif	6.47e-22	0.029	0.971	4cell	4cell	3.943
4cell/t113_s09_c10_ij.tif	8.07e-22	0.107	0.893	4cell	4cell	3.786
4cell/t114_s01_c06_ij.tif	3.62e-22	0.037	0.963	4cell	4cell	3.927
4cell/t114_s08_c07_ij.tif	7.28e-22	0.045	0.955	4cell	4cell	3.911
4cell/t115_s02_c12_ij.tif	2.79e-22	0.007	0.993	4cell	4cell	3.987
4cell/t117_s05_c05_ij.tif	1.2e-21	0.026	0.974	4cell	4cell	3.949
4cell/t118_s04_c12_ij.tif	8.57e-22	0.050	0.950	4cell	4cell	3.899
4cell/t120_s06_c05_ij.tif	8.67e-22	0.079	0.921	4cell	4cell	3.843
4cell/t120_s06_c06_ij.tif	3.12e-22	0.053	0.947	4cell	4cell	3.894
4cell/t120_s06_c12_ij.tif	6.28e-22	0.268	0.732	4cell	4cell	3.464
4cell/t120_s12_c02_ij.tif	2.6e-22	0.173	0.827	4cell	4cell	3.655
4cell/t120_s12_c06_ij.tif	8.17e-22	0.007	0.993	4cell	4cell	3.986
4cell/t120_s12_c07_ij.tif	1.23e-21	0.012	0.988	4cell	4cell	3.976
4cell/t123_s04_c02_ij.tif	2.66e-22	0.181	0.819	4cell	4cell	3.639
4cell/t128_s03_c11_ij.tif	7.28e-22	0.004	0.996	4cell	4cell	3.993
4cell/t130_s02_c01_ij.tif	5.31e-22	0.004	0.996	4cell	4cell	3.992
4cell/t130_s05_c06_ij.tif	9.81e-22	0.011	0.989	4cell	4cell	3.979
4cell/t130_s05_c09_ij.tif	6.56e-22	0.038	0.962	4cell	4cell	3.925
4cell/t130_s07_c08_ij.tif	3.25e-22	0.249	0.751	4cell	4cell	3.502
4cell/t130_s07_c11_ij.tif	6.79e-22	0.251	0.749	4cell	4cell	3.497
4cell/t130_s08_c05_ij.tif	2.16e-21	0.011	0.989	4cell	4cell	3.979
4cell/t130_s09_c07_ij.tif	6.51e-22	0.019	0.981	4cell	4cell	3.961
4cell/t130_s10_c01_ij.tif	7.4e-22	0.117	0.883	4cell	4cell	3.766
4cell/t138_s03_c02_ij.tif	5.04e-22	0.007	0.993	4cell	4cell	3.987
4cell/t140_s01_c10_ij.tif	7.8e-22	0.009	0.991	4cell	4cell	3.982
4cell/t140_s04_c01_ij.tif	2.23e-21	0.009	0.991	4cell	4cell	3.981
4cell/t1_s01_c05_ij.tif	4.11e-22	0.009	0.991	4cell	4cell	3.982
4cell/t1_s01_c10_ij.tif	7.58e-22	0.005	0.995	4cell	4cell	3.989
4cell/t1_s02_c11_ij.tif	1.15e-21	0.036	0.964	4cell	4cell	3.928
4cell/t1_s03_c08_ij.tif	9.55e-23	0.078	0.922	4cell	4cell	3.844
4cell/t1_s04_c04_ij.tif	4.73e-22	0.004	0.996	4cell	4cell	3.993
4cell/t1_s05_c06_ij.tif	2.11e-22	0.006	0.994	4cell	4cell	3.988
4cell/t1_s07_c08_ij.tif	1.11e-21	0.078	0.922	4cell	4cell	3.845
4cell/t1_s09_c01_ij.tif	5.99e-22	0.010	0.990	4cell	4cell	3.980
4cell/t1_s09_c11_ij.tif	7.1e-22	0.130	0.870	4cell	4cell	3.740
4cell/t1_s10_c08_ij.tif	6.98e-22	0.034	0.966	4cell	4cell	3.932
4cell/t1_s13_c04_ij.tif	1.08e-21	0.021	0.979	4cell	4cell	3.958
4cell/t1_s14_c06_ij.tif	1.68e-21	0.038	0.962	4cell	4cell	3.923
END

	return ( split (/\n/,$expected_out) );
}
