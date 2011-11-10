#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "FeatureGroup.h"
#include "FeatureAlgorithm.h"
#include "Channel.h"
#include "transforms.h"

#define DEBUG 0

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


WNDCHRM_ERROR FeatureGroup::get_name( string& out_str ) {
	if( !name.empty() ) {
		out_str = name;
		return WC_NO_ERROR;
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
	return WC_NO_ERROR;
}

WNDCHRM_ERROR FeatureGroup::calculate_coefficients(MatrixMap& saved_pixel_planes, std::vector<double> &coeffs )
{
	return algorithm->calculate( saved_pixel_planes, transforms, coeffs );
}

