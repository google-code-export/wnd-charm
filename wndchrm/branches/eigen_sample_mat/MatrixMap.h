#ifndef __MATRIX_MAP_H__
#define __MATRIX_MAP_H__


#include <vector>
#include <map> //needed for datatype MatrixMap

//typedef std::map< TransformList, ImageMatrix* > MatrixMap;
// CEC_const typedef vector<Transform const *> TransformList;

enum MatrixMapError {
	MM_UNINITIALIZED,
	MM_NO_ERROR,
	MM_FAIL_NULL_POINTER,
	MM_FAIL_RECURSIVE_CALL,
	MM_TRANSFORM_FAIL,
	MM_EMPTY
};
	
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

		MatrixMapError save_transform( std::vector<Transform *> &sequence, ImageMatrix * in_matr );
		MatrixMapError obtain_transform( std::vector<Transform *> &sequence, ImageMatrix ** out_matr );
	private:
		MapType m_map;
};

#endif
