#!/bin/sh

# wndchrm nightly build / smoke test script
# by Chris Coletta

# This script is kicked off by cron on behalf of the user tester@iicbu2 at midnight.
# You can edit the cron settings by logging into iicbu2 as "tester" and typing `crontab -e`
# (make sure you have exported the environment variable 'EDITOR=vi' or emacs or nano etc.)

# This script looks for a file called /home/tester/build_control_flags/kick_me_for_wndchrm_build.
# If it doesn't exist, it is created, and a build/test sequence begins.
# If is does exist, the modification date on the file is compared to the modification date 
# of the file ~/build_control_flags/last_wndchrm_build_completed. If the "kick me" flag has been 
# modified more recently than the "last build completed flag" the build/test sequence is initiated.
# Otherwise, this script exits.

# the 'kick_me_for_wndchrm_build' can be

set -v
set -x
NOW=`date '+%Y-%m-%d_%I-%M%p'`
WNDCHRM_REPOS_LOCATION="file:///srv/svn/repositories/wndchrm"
BASE_DIR="/home/tester"
TEST_DIR="${BASE_DIR}/test_products/wndchrm_$NOW"
CHECKOUT_DIR="${TEST_DIR}/checkout_area"
COMPILE_DIR="${TEST_DIR}/compile_area"
# Note: anything having to do with running a smoke test should go into the
# wndchrm_smoke_test.pl file run at the end of this script
#  TEST_FEATURE_WEIGHT_FILE="${BASE_DIR}/read_only_files/terminalbulb_feature_set_CONTROL"
# anyfile that wndchrm produces should go in here, including results from running 
# wndchrm test
OUTPUT_PRODUCTS_DIR="${TEST_DIR}/output_products"
# The results of this build/test process should go in here.
TEST_RESULTS_DIR="${TEST_DIR}/test_results"

echo "*****************************************************"
echo "  WND-CHRM build system" 
echo "  Performed on $NOW "
echo ""
echo "Creating test directory..."
mkdir $TEST_DIR
echo "Creating checkout directory..."
mkdir $CHECKOUT_DIR
echo "Checking out wndchrm source tree HEAD from svn..."
cd $CHECKOUT_DIR
# Need to specify full path to locally compiled svn
# since cron runs this command and the paths aren't
# set the same way when you are logged in via a terminal.
/usr/local/bin/svn co ${WNDCHRM_REPOS_LOCATION}/trunk/
cd trunk
./configure
make dist
export TARBALL_NAME=`ls *.tar.gz`
TARBALL_BASENAME=`echo $TARBALL_NAME | sed -r "s/\\.tar\\.gz//g"` 
TARBALL_RENAMED="${TARBALL_BASENAME}-$NOW"
TARBALL_RENAMED_W_EXTN="${TARBALL_RENAMED}.tar.gz"
mkdir $COMPILE_DIR
mv $TARBALL_NAME ${COMPILE_DIR}/$TARBALL_RENAMED_W_EXTN
cd ${COMPILE_DIR}/
tar zxvf $TARBALL_RENAMED_W_EXTN
cd ${TARBALL_BASENAME}
./configure
make
touch /home/tester/build_control_flags/last_completed_wndchrm_build 


# TODO: create make target instead of calling script directly
mkdir $OUTPUT_PRODUCTS_DIR
mkdir $TEST_RESULTS_DIR
cp $BASE_DIR/scripts/wndchrm_smoke_test.pl .
TEST_RESULTS_FILE="${TEST_RESULTS_DIR}/${NOW}_test_results"
# Run the smoke test. The return value of the test script is captured in $?
# 0 = passing, > 0 = failing
./wndchrm_smoke_test.pl --suffix=$NOW --basedir=$TEST_DIR > $TEST_RESULTS_FILE
if test $? -gt 0
then
  # make a message and mail it to the appropriate parties
  mail -s "failed wndchrm autobuild for $NOW" colettace@mail.nih.gov < $TEST_RESULTS_FILE
else
  mail -s "successful wndchrm autobuild for $NOW" colettace@mail.nih.gov < $TEST_RESULTS_FILE
fi

