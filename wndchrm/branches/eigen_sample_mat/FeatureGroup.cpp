#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "FeatureGroup.h"
#include "FeatureAlgorithm.h"
#include "Channel.h"
#include "transforms.h"

#define DEBUG 1

void FeatureGroup::print_info() const {
	//std::cout << ", group name:\t" << name;// << std::endl;
	if( algorithm )
		algorithm->print_info();
	if( channel )
		channel->print_info();
	unsigned int i;
	for( i = 0; i < transforms.size(); ++i ) {
		std::cout << "\t";
		transforms[i]->print_info();
	}
}

/*
int FeatureGroup::get_required_transforms(string& req_transforms) const {
	req_transforms.clear();
	vector<Transform const*>::iterator it = transforms.begin();

	for( ; it != transforms.end(); it++ )
		req_transforms += *it;
	*/



int FeatureGroup::get_name( string& out_str ) {
	if( !name.empty() ) {
		out_str = name;
		return 1;
	}
	std::ostringstream oss;
	string temp;
	if( (temp = algorithm->name).empty() )
		temp = "Unnamed Algorithm";
 
	oss << temp << " (";
	//for( vector<Transform const *>::iterator t_it = transforms.begin(); 
	for( vector<Transform *>::iterator t_it = transforms.begin(); 
			 t_it != transforms.end(); ++t_it ) {
		if( (temp = (*t_it)->name).empty() )
			temp = "Unnamed Transform";
		oss << " " << temp << " (";
	}
	for( int i = 0; i < transforms.size(); ++i )
		oss << ") ";

	oss << ")";

	name = oss.str();
	out_str = name;
	return 1;
}

//==========================================================================
/*ImageMatrix * FeatureGroup::obtain_transform(
		MatrixMap &saved_pixel_planes,
		vector<Transform const *> sequence ) const */
ImageMatrix * FeatureGroup::obtain_transform(
		MatrixMap &saved_pixel_planes,
		vector<Transform *> sequence )
{
	int retval = 0;
#if DEBUG
	std::cout << "\tFeatureGroup::obtain_transform:" << std::endl;
	std::cout << "\t\tMatrixMap size =" << saved_pixel_planes.size() << std::endl;
	std::cout << "\t\tsequence size =" << sequence.size() << std::endl;
	std::cout << "\t\tsequence to be obtained: ";
	for( vector<Transform*>::iterator seq_it = sequence.begin(); seq_it != sequence.end(); ++seq_it )
	{
		std::cout << (*seq_it)->name << " ";
	}
	std::cout << std::endl;
#endif
	MatrixMap::iterator t_it = saved_pixel_planes.find(sequence);
	if( t_it != saved_pixel_planes.end() ) {
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
		return ( t_it->second );
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

	// Step 2a.
	t_it = saved_pixel_planes.find(sequence);
	if( t_it != saved_pixel_planes.end() )
	{
#if DEBUG
		std::cout << "FeatureGroup::obtain_transform: found the shortened sequence in the MatrixMap" << std::endl;
#endif
		intermediate_pixel_plane = t_it->second;
	}
	else {
		// Step 2b.
#if DEBUG
		std::cout << "FeatureGroup::obtain_transform: couldn't find shortened sequence in the MatrixMap" << std::endl;
#endif
		// recursion call here:
		intermediate_pixel_plane = obtain_transform(saved_pixel_planes, sequence);
		if( NULL == intermediate_pixel_plane ) {
			std::cout << "FeatureGroup::obtain_transform: Call to obtain transform for shortened sequence returned null pixel plane"
				<< std::endl;
		}
		// no need to save the intermediate --
		// If it generates a transform, obtain_transform will save the pixel plane it returns as the last step
	}

	// Step 3.
	if( NULL == intermediate_pixel_plane ) {
		std::cout << "FeatureGroup::obtain_transform: skipping transform since intermediate pixel plane is null." << std::endl;
		return NULL;
	}
	retval = 
		last_transform_in_sequence->transform( intermediate_pixel_plane, &output_pixel_plane );
#if DEBUG
	std::cout << "FeatureGroup::obtain_transform: Return value from transform is " << retval << std::endl;
#endif

	// Step 4.
	if( !( retval < 0 ) && (NULL != output_pixel_plane) )
		saved_pixel_planes[original_sequence] = output_pixel_plane;

	return output_pixel_plane;
}


