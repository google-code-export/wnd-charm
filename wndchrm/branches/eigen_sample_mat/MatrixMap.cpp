#include <iostream>
#include "MatrixMap.h"
#include "cmatrix.h"
#include "transforms.h"
#include "wndchrm_error.h"

#define DEBUG 1

//==========================================================================
MatrixMap::MatrixMap( ImageMatrix* untransformed_matrix )
{
	if( NULL != untransformed_matrix )
	{
	// Load the untransformed pixel field into the transform map
	// Key: a vector of Transforms, length 0. Value, the corresponding ImageMatrix

	vector<Transform *> blank_transform_list;
  save_transform( blank_transform_list, untransformed_matrix );
	}
}

//==========================================================================

MatrixMap::~MatrixMap() {
	for( MapType::iterator it = m_map.begin(); it != m_map.end(); it++ ) {
		delete it->second;
		it->second = NULL;
	}
}
//==========================================================================
WNDCHRM_ERROR MatrixMap::save_transform( std::vector<Transform *> &sequence, ImageMatrix * in_matr )
{
	m_map[sequence] = in_matr;
	return WC_NO_ERROR;
}

//==========================================================================
WNDCHRM_ERROR MatrixMap::obtain_transform( vector<Transform *> &sequence, ImageMatrix ** out_matr )
{
	if( m_map.size() < 1 )
		return WC_EMPTY;

#if DEBUG
	std::cout << "\tMatrixMap::obtain_transform:" << std::endl;
	std::cout << "\t\tMatrixMap size =" << m_map.size() << std::endl;
	std::cout << "\t\tsequence size =" << sequence.size() << std::endl;
	std::cout << "\t\tsequence to be obtained: ";
	for( vector<Transform*>::iterator seq_it = sequence.begin(); seq_it != sequence.end(); ++seq_it )
	{
		std::cout << (*seq_it)->name << " ";
	}
	std::cout << std::endl;
#endif
	MapType::iterator t_it = m_map.find(sequence);
	if( t_it != m_map.end() ) {
#if DEBUG
		std::cout << "\t\tFound transform ";
		std::pair< std::vector< Transform* > , ImageMatrix* > temp_pair = *t_it;
		std::vector<Transform*> seq_in_matrixmap = temp_pair.first;
		for( vector<Transform*>::iterator seq2_it = seq_in_matrixmap.begin(); seq2_it != seq_in_matrixmap.end(); ++seq2_it )
		{
			std::cout << (*seq2_it)->name << " ";
		}
		std::cout << std::endl;
#endif
		*out_matr = t_it->second;
		return WC_NO_ERROR;
	}

	// If we've gotten here, the requested sequence of transforms does not exist
	// in the intermediate storage map "saved_pixel_planes"
	// We now will start to parse the work order contained in the vector "sequence"
	// Step 1: Strip off the last transform in the sequence and call it "last_transform_in_sequence"
	// Step 2a: Check to see if "saved_pixel_planes" contains the ImageMatrix 
	//         corresponding to the shortened sequence, called the "intermediate_pixel_plane"
	// Step 2b: If no, recursively call this function, obtain_transform, using
	//         the shortened sequence to get the intermediate_pixel_plane
	// Step 3. transform the "intermediate_pixel_plane" using the "last_transform_in_sequence"
	// Step 4: Save the resulting transformed pixel plane from Step 3 in "saved_pixel_planes"
	// Step 5: Return.

	#if DEBUG
	std::cout << "\t\tsequence not found" << std::endl;
	#endif

	// Step 1.
	ImageMatrix* output_pixel_plane = NULL;
	ImageMatrix* intermediate_pixel_plane = NULL;

	Transform* last_transform_in_sequence = sequence.back();
	#if DEBUG
	std::cout << "\t\tlast tform in sequence is " << last_transform_in_sequence->name << std::endl;
	#endif
	vector<Transform *> original_sequence = sequence;
	sequence.pop_back();

	WNDCHRM_ERROR retval = WC_UNINITIALIZED;
	// Step 2a.
	t_it = m_map.find(sequence);
	if( t_it != m_map.end() )
	{
		#if DEBUG
		std::cout << "MatrixMap::obtain_transform: found the shortened sequence in the MatrixMap" << std::endl;
		#endif
		intermediate_pixel_plane = t_it->second;
		if( NULL == intermediate_pixel_plane ) {
			std::cout << "MatrixMap::obtain_transform: skipping transform since intermediate pixel plane is null." << std::endl;
			return WC_FAIL_NULL_POINTER;
		}
	}
	else {
		// Step 2b.

		#if DEBUG
		std::cout << "MatrixMap::obtain_transform: couldn't find shortened sequence in the MatrixMap" << std::endl;
		#endif

		// recursion call here:
		retval = obtain_transform(sequence, &intermediate_pixel_plane);
		if( NULL == intermediate_pixel_plane ) {
			std::cout << "MatrixMap::obtain_transform: Call to obtain transform for shortened sequence returned null pixel plane"
				<< std::endl;
			return WC_FAIL_RECURSIVE_CALL;
		}
		// no need to save the intermediate --
		// If it generates a transform, obtain_transform will save the pixel plane it returns as the last step
	}

	// Step 3.
	int tform_retval = 
		last_transform_in_sequence->transform( intermediate_pixel_plane, &output_pixel_plane );
	#if DEBUG
	std::cout << "MatrixMap::obtain_transform: Return value from transform is "
	          << tform_retval << std::endl;
	#endif

	// Step 4.
	if( ( retval > 0 ) || ( NULL == output_pixel_plane ) )
	{
		std::cout << "ERROR: Call to transform "
		          << last_transform_in_sequence->name
		          << " returned null pixel plane." << std::endl;
		return WC_TRANSFORM_FAIL;
	}

	m_map[original_sequence] = output_pixel_plane;
	*out_matr = output_pixel_plane;
	return WC_NO_ERROR;
}


