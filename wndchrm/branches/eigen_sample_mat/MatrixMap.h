#ifndef __MATRIX_MAP_H__
#define __MATRIX_MAP_H__


#include <vector>
#include <map> //needed for datatype MatrixMap
#include "wndchrm_error.h"

//typedef std::map< TransformList, ImageMatrix* > MatrixMap;
// CEC_const typedef vector<Transform const *> TransformList;

	
class Transform;
class ImageMatrix;

typedef std::vector< Transform* > TransformList;
typedef std::map< TransformList, ImageMatrix* > MapType;

class MatrixMap
{
	public:
		MatrixMap() {};
		MatrixMap( ImageMatrix* untransformed_matrix );

		//Destructor should be the one to delete the ImageMatrices
		~MatrixMap();

		WNDCHRM_ERROR save_transform( std::vector<Transform *> &sequence, ImageMatrix * in_matr );
		WNDCHRM_ERROR obtain_transform( std::vector<Transform *> &sequence, ImageMatrix ** out_matr );
	private:
		MapType m_map;
};

#endif
