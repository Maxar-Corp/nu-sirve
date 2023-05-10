
#include "Euler.h"

namespace rotate{

	/*==============================================================================
	* Function: rotate::CoordFrame_Rotation1
	*
	* Purpose: Function calculates the transformation matrix to rotate a coordinate
	*			about its primary axis
	*
	* Inputs: angle -- angle (in degrees) for the rotation
	*
	*
	* Return: Returns a 3 x 3 transformation matrix. Assumes a format of:
	*			[Rotation Matrix] * vector
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	arma::mat CoordFrame_Rotation1(double angle) {

		arma::mat rotate(3, 3);
		angle = M_PI / 180 * angle;

		rotate(0, 0) = 1;
		rotate(1, 0) = 0;
		rotate(2, 0) = 0;
		rotate(0, 1) = 0;
		rotate(1, 1) = cos(angle);
		rotate(2, 1) = -sin(angle);
		rotate(0, 2) = 0;
		rotate(1, 2) = sin(angle);
		rotate(2, 2) = cos(angle);

		return rotate;
	}

	/*==============================================================================
	* Function: rotate::CoordFrame_Rotation1
	*
	* Purpose: Function calculates the transformation matrix to rotate a coordinate
	*			about its primary axis
	*
	* Inputs: angle -- a vector list of angles (in degrees) for the rotation
	*
	*
	* Return: Returns a 9 x n transformation matrix. Assumes a format of
	*			column first for ordering the matrix
	*
	* Notes: None
	*=============================================================================*/
	arma::mat CoordFrame_Rotation1(arma::vec angle) {

		int numValues;

		numValues = angle.n_elem;

		arma::mat rotate(9, numValues);
		angle = M_PI / 180 * angle;

		rotate.row(0) = (arma::ones(angle.n_elem)).t();
		rotate.row(1) = (arma::zeros(angle.n_elem)).t();
		rotate.row(2) = (arma::zeros(angle.n_elem)).t();
		rotate.row(3) = (arma::zeros(angle.n_elem)).t();
		rotate.row(4) = cos(angle).t();
		rotate.row(5) = -sin(angle).t();
		rotate.row(6) = (arma::zeros(angle.n_elem)).t();
		rotate.row(7) = sin(angle).t();
		rotate.row(8) = cos(angle).t();

		return rotate;
	}

	/*==============================================================================
	* Function: rotate::CoordFrame_Rotation2
	*
	* Purpose: Function calculates the transformation matrix to rotate a coordinate
	*			about its secondary axis
	*
	* Inputs: angle -- angle (in degrees) for the rotation
	*
	*
	* Return: Returns a 3 x 3 transformation matrix. Assumes a format of:
	*			[Rotation Matrix] * vector
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	arma::mat CoordFrame_Rotation2(double angle) {

		arma::mat rotate(3, 3);
		angle = M_PI / 180 * angle;

		rotate(0, 0) = cos(angle);
		rotate(1, 0) = 0;
		rotate(2, 0) = sin(angle);
		rotate(0, 1) = 0;
		rotate(1, 1) = 1;
		rotate(2, 1) = 0;
		rotate(0, 2) = -sin(angle);
		rotate(1, 2) = 0;
		rotate(2, 2) = cos(angle);

		return rotate;
	}

	/*==============================================================================
	* Function: rotate::CoordFrame_Rotation2
	*
	* Purpose: Function calculates the transformation matrix to rotate a coordinate
	*			about its secondary axis
	*
	* Inputs: angle -- a vector list of angles (in degrees) for the rotation
	*
	*
	* Return: Returns a 9 x n transformation matrix. Assumes a format of
	*			column first for ordering the matrix
	*
	* Notes: None
	*=============================================================================*/
	arma::mat CoordFrame_Rotation2(arma::vec angle) {

		int numValues;

		numValues = angle.n_elem;

		arma::mat rotate(9, numValues);
		angle = M_PI / 180 * angle;

		rotate.row(0) = cos(angle).t();
		rotate.row(1) = (arma::zeros(angle.n_elem)).t();
		rotate.row(2) = sin(angle).t();
		rotate.row(3) = (arma::zeros(angle.n_elem)).t();
		rotate.row(4) = (arma::ones(angle.n_elem)).t();
		rotate.row(5) = (arma::zeros(angle.n_elem)).t();
		rotate.row(6) = -sin(angle).t();
		rotate.row(7) = (arma::zeros(angle.n_elem)).t();
		rotate.row(8) = cos(angle).t();

		return rotate;
	}

	/*==============================================================================
	* Function: rotate::CoordFrame_Rotation3
	*
	* Purpose: Function calculates the transformation matrix to rotate a coordinate
	*			about its tertiary axis
	*
	* Inputs: angle -- angle (in degrees) for the rotation
	*
	*
	* Return: Returns a 3 x 3 transformation matrix. Assumes a format of:
	*			[Rotation Matrix] * vector
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	arma::mat CoordFrame_Rotation3(double angle) {

		arma::mat rotate(3, 3);
		angle = M_PI / 180 * angle;

		rotate(0, 0) = cos(angle);
		rotate(1, 0) = -sin(angle);
		rotate(2, 0) = 0;
		rotate(0, 1) = sin(angle);
		rotate(1, 1) = cos(angle);
		rotate(2, 1) = 0;
		rotate(0, 2) = 0;
		rotate(1, 2) = 0;
		rotate(2, 2) = 1;

		return rotate;
	}

	/*==============================================================================
	* Function: rotate::CoordFrame_Rotation1
	*
	* Purpose: Function calculates the transformation matrix to rotate a coordinate
	*			about its tertiary axis
	*
	* Inputs: angle -- a vector list of angles (in degrees) for the rotation
	*
	*
	* Return: Returns a 9 x n transformation matrix. Assumes a format of
	*			column first for ordering the matrix
	*
	* Notes: None
	*=============================================================================*/
	arma::mat CoordFrame_Rotation3(arma::vec angle) {

		int numValues;

		numValues = angle.n_elem;

		arma::mat rotate(9, numValues);
		angle = M_PI / 180 * angle;

		rotate.row(0) = cos(angle).t();
		rotate.row(1) = -sin(angle).t();
		rotate.row(2) = (arma::zeros(angle.n_elem)).t();
		rotate.row(3) = sin(angle).t();
		rotate.row(4) = cos(angle).t();
		rotate.row(5) = (arma::zeros(angle.n_elem)).t();
		rotate.row(6) = (arma::zeros(angle.n_elem)).t();
		rotate.row(7) = (arma::zeros(angle.n_elem)).t();
		rotate.row(8) = (arma::ones(angle.n_elem)).t();

		return rotate;
	}

	/*==============================================================================
	* Function: rotate::MatrixMulitply
	*
	* Purpose: Multiplies a transformation matrix that is rerpesented by an 9 x n
				matrix by a secondary matrix. The secondary matrix can be another
				9 x n representing several 3 x 3 transformation matrices  or a 3 x n
				representing a series of vectors
	*
	* Inputs: array1 -- a 9 x n cordinate transformation matrix
	*		  array2 -- another 9 x n coordinate transformation matrix or a 3 x n list of vectors
	*	   transpose -- a boolean to determine if the first 9 x n matrix should be transposed
	*
	*
	* Return: Returns either a new 9 x n coordinate transformation matrix or a
	*			3 x n matrix representing the new vector coordinates
	*
	* Notes: None
	*=============================================================================*/
	arma::mat MatrixMulitply(arma::mat array1, arma::mat array2, bool transpose) {

		arma::mat result;

		int numRows1, numCols1, numRows2, numCols2;
		numRows1 = array1.n_rows;
		numCols1 = array1.n_cols;

		numRows2 = array2.n_rows;
		numCols2 = array2.n_cols;

		if (numRows2 == 3) { //Coordinate transform multiplied by a list of vectors
			result.set_size(3, numCols2);
			if (numCols1 < numCols2) {
				arma::mat tmp(array1.n_rows, array2.n_cols); tmp.zeros();
				tmp.row(0) += array1(0, 0); tmp.row(1) += array1(1, 0); tmp.row(2) += array1(2, 0);
				tmp.row(3) += array1(3, 0); tmp.row(4) += array1(4, 0); tmp.row(5) += array1(5, 0);
				tmp.row(6) += array1(6, 0); tmp.row(7) += array1(7, 0); tmp.row(8) += array1(8, 0);
				array1 = tmp;
			}
			if (numCols2 < numCols1) {
				arma::mat tmp(array2.n_rows, array1.n_cols); tmp.zeros();
				tmp.row(0) += array2(0, 0); tmp.row(1) += array2(1, 0); tmp.row(2) += array2(2, 0);
				//tmp.row(3) += array2(3, 0); tmp.row(4) += array2(4, 0); tmp.row(5) += array2(5, 0);
				//tmp.row(6) += array2(6, 0); tmp.row(7) += array2(7, 0); tmp.row(8) += array2(8, 0);
				array2 = tmp;
			}


			if (transpose) {
				result.row(0) = array1.row(0) % array2.row(0) + array1.row(1) % array2.row(1) + array1.row(2) % array2.row(2);
				result.row(1) = array1.row(3) % array2.row(0) + array1.row(4) % array2.row(1) + array1.row(5) % array2.row(2);
				result.row(2) = array1.row(6) % array2.row(0) + array1.row(7) % array2.row(1) + array1.row(8) % array2.row(2);
			}
			else {
				result.row(0) = array1.row(0) % array2.row(0) + array1.row(3) % array2.row(1) + array1.row(6) % array2.row(2);
				result.row(1) = array1.row(1) % array2.row(0) + array1.row(4) % array2.row(1) + array1.row(7) % array2.row(2);
				result.row(2) = array1.row(2) % array2.row(0) + array1.row(5) % array2.row(1) + array1.row(8) % array2.row(2);
			}
		}
		else if (numRows2 == 9) { //Coordinate transform multiplied by other coordinate transforms
			result.set_size(9, numCols2);
			if (numCols1 < numCols2) {
				arma::mat tmp(numRows2, array2.n_cols); tmp.zeros();
				tmp.row(0) += array1(0, 0); tmp.row(1) += array1(1, 0); tmp.row(2) += array1(2, 0);
				tmp.row(3) += array1(3, 0); tmp.row(4) += array1(4, 0); tmp.row(5) += array1(5, 0);
				tmp.row(6) += array1(6, 0); tmp.row(7) += array1(7, 0); tmp.row(8) += array1(8, 0);
				array1 = tmp;
			}
			if (numCols2 < numCols1) {
				arma::mat tmp(numRows2, array1.n_cols); tmp.zeros();
				tmp.row(0) += array2(0, 0); tmp.row(1) += array2(1, 0); tmp.row(2) += array2(2, 0);
				tmp.row(3) += array2(3, 0); tmp.row(4) += array2(4, 0); tmp.row(5) += array2(5, 0);
				tmp.row(6) += array2(6, 0); tmp.row(7) += array2(7, 0); tmp.row(8) += array2(8, 0);
				array2 = tmp;
			}

			if (transpose) {
				result.row(0) = array1.row(0) % array2.row(0) + array1.row(1) % array2.row(1) + array1.row(2) % array2.row(2);
				result.row(1) = array1.row(3) % array2.row(0) + array1.row(4) % array2.row(1) + array1.row(5) % array2.row(2);
				result.row(2) = array1.row(6) % array2.row(0) + array1.row(7) % array2.row(1) + array1.row(8) % array2.row(2);

				result.row(3) = array1.row(0) % array2.row(3) + array1.row(1) % array2.row(4) + array1.row(2) % array2.row(5);
				result.row(4) = array1.row(3) % array2.row(3) + array1.row(4) % array2.row(4) + array1.row(5) % array2.row(5);
				result.row(5) = array1.row(6) % array2.row(3) + array1.row(7) % array2.row(4) + array1.row(8) % array2.row(5);

				result.row(6) = array1.row(0) % array2.row(6) + array1.row(1) % array2.row(7) + array1.row(2) % array2.row(8);
				result.row(7) = array1.row(3) % array2.row(6) + array1.row(4) % array2.row(7) + array1.row(5) % array2.row(8);
				result.row(8) = array1.row(6) % array2.row(6) + array1.row(7) % array2.row(7) + array1.row(8) % array2.row(8);

			}
			else {
				result.row(0) = array1.row(0) % array2.row(0) + array1.row(3) % array2.row(1) + array1.row(6) % array2.row(2);
				result.row(1) = array1.row(1) % array2.row(0) + array1.row(4) % array2.row(1) + array1.row(7) % array2.row(2);
				result.row(2) = array1.row(2) % array2.row(0) + array1.row(5) % array2.row(1) + array1.row(8) % array2.row(2);

				result.row(3) = array1.row(0) % array2.row(3) + array1.row(3) % array2.row(4) + array1.row(6) % array2.row(5);
				result.row(4) = array1.row(1) % array2.row(3) + array1.row(4) % array2.row(4) + array1.row(7) % array2.row(5);
				result.row(5) = array1.row(2) % array2.row(3) + array1.row(5) % array2.row(4) + array1.row(8) % array2.row(5);

				result.row(6) = array1.row(0) % array2.row(6) + array1.row(3) % array2.row(7) + array1.row(6) % array2.row(8);
				result.row(7) = array1.row(1) % array2.row(6) + array1.row(4) % array2.row(7) + array1.row(7) % array2.row(8);
				result.row(8) = array1.row(2) % array2.row(6) + array1.row(5) % array2.row(7) + array1.row(8) % array2.row(8);

			}
		}
		else {
			throw std::invalid_argument("Dimensions Do Not Match in MatrixMultiply");
		}

		return result;
	}

	/*==============================================================================
	* Function: rotate::MCross
	*
	* Purpose: Function calculates the cross product for a list of vectors
	*
	* Inputs: array1 -- a 3 x n list of vectors
	*		  array2 -- a 3 x n list of vectors
	*
	*
	* Return: Returns a 3 x n list of vectors representing the cross product
	*
	* Notes: None
	*=============================================================================*/
	arma::mat MCross(arma::mat array1, arma::mat array2) {

		int numRows1, numCols1, numRows2, numCols2;
		numRows1 = array1.n_rows;
		numCols1 = array1.n_cols;

		numRows2 = array2.n_rows;
		numCols2 = array2.n_cols;

		if (numCols1 == 1 && numCols2 > 1) {
			arma::vec tmp = array1;
			arma::mat tmp2(3, numCols2);
			arma::rowvec zeros(numCols2, arma::fill::zeros);
			tmp2.row(0) = zeros + tmp(0);
			tmp2.row(1) = zeros + tmp(1);
			tmp2.row(2) = zeros + tmp(2);

			array1 = tmp2;
		}

		if (numCols2 == 1 && numCols1 > 1) {
			arma::vec tmp = array2;
			arma::mat tmp2(3, numCols1);
			arma::rowvec zeros(numCols1, arma::fill::zeros);
			tmp2.row(0) = zeros + tmp(0);
			tmp2.row(1) = zeros + tmp(1);
			tmp2.row(2) = zeros + tmp(2);

			array2 = tmp2;
		}

		numCols1 = array1.n_cols;
		numCols2 = array2.n_cols;

		if (numCols1 != numCols2) {
			throw std::invalid_argument("Number of Columns Does Not Match in MCross");
		}

		arma::mat output(3, numCols2);
		output.row(0) = array1.row(1) % array2.row(2) - array1.row(2) % array2.row(1);
		output.row(1) = array1.row(2) % array2.row(0) - array1.row(0) % array2.row(2);
		output.row(2) = array1.row(0) % array2.row(1) - array1.row(1) % array2.row(0);

		return output;
	}

	/*arma::vec MCross(arma::vec array1, arma::vec array2) {

		return arma::cross(array1, array2);
	}*/

	/*==============================================================================
	* Function: rotate::MDot
	*
	* Purpose: Function calculates the dot product for a list of vectors
	*
	* Inputs: array1 -- a 3 x n list of vectors
	*		  array2 -- a 3 x n list of vectors
	*
	*
	* Return: Returns a vector holding the dot product very each vector pair
	*
	* Notes: None
	*=============================================================================*/
	arma::mat MDot(arma::mat array1, arma::mat array2) {

		int numRows1, numCols1, numRows2, numCols2;
		numRows1 = array1.n_rows;
		numCols1 = array1.n_cols;

		numRows2 = array2.n_rows;
		numCols2 = array2.n_cols;

		if (numCols1 == 1 && numCols2 > 1) {
			arma::vec tmp = array1;
			arma::mat tmp2(3, numCols2);
			arma::rowvec zeros(numCols2, arma::fill::zeros);
			tmp2.row(0) = zeros + tmp(0);
			tmp2.row(1) = zeros + tmp(1);
			tmp2.row(2) = zeros + tmp(2);

			array1 = tmp2;
		}

		if (numCols2 == 1 && numCols1 > 1) {
			arma::vec tmp = array2;
			arma::mat tmp2(3, numCols1);
			arma::rowvec zeros(numCols1, arma::fill::zeros);
			tmp2.row(0) = zeros + tmp(0);
			tmp2.row(1) = zeros + tmp(1);
			tmp2.row(2) = zeros + tmp(2);

			array2 = tmp2;
		}

		numCols1 = array1.n_cols;
		numCols2 = array2.n_cols;

		if (numCols1 != numCols2) {
			throw std::invalid_argument("Number of Columns Does Not Match in MCross");
		}

		arma::mat output(3, numCols2);

		output = array1.row(0) % array2.row(0);
		output += array1.row(1) % array2.row(1);
		output += array1.row(2) % array2.row(2);

		return output;
	}

	/*double MDot(arma::vec array1, arma::vec array2) {

		return arma::dot(array1, array2);
	}*/

	/*==============================================================================
	* Function: rotate::MMagnitude
	*
	* Purpose: Function calculates the magnitude for a list of vectors
	*
	* Inputs: array1 -- a 3 x n list of vectors
	*
	*
	* Return: Returns a vector holding the magnitude of the list of vectors
	*
	* Notes: None
	*=============================================================================*/
	arma::mat MMagnitude(arma::mat array1) {

		int numRows1, numCols1;
		numRows1 = array1.n_rows;
		numCols1 = array1.n_cols;

		//arma::vec output(numCols1);

		arma::mat output = arma::square(array1.row(0)) + arma::square(array1.row(1)) + arma::square(array1.row(2));
		output = arma::sqrt(output);

		//output = sqrtValues.row(0);

		return output.t();
	}

	/*double MMagnitude(arma::vec array1) {

		double output;

		output = array1(0) * array1(0) + array1(1) * array1(1) + array1(2) * array1(2);
		output = std::sqrt(output);

		return output;
	}*/

	/*==============================================================================
	* Function: rotate::MUnit
	*
	* Purpose: Function calculates the unit vector for a list of vectors
	*
	* Inputs: array1 -- a 3 x n list of vectors
	*
	*
	* Return: Returns a 3 x n list of vectors representing the unit vector
	*
	* Notes: None
	*=============================================================================*/
	arma::mat MUnit(arma::mat array1) {

		arma::mat mag, unit;

		mag = MMagnitude(array1);

		unit = array1;

		unit.row(0) = unit.row(0) / mag.t();
		unit.row(1) = unit.row(1) / mag.t();
		unit.row(2) = unit.row(2) / mag.t();

		return unit;
	}

	/*arma::vec MUnit(arma::vec array1) {

		arma::vec unit;
		double mag;

		mag = MMagnitude(array1);
		if (mag == 0) return array1;
		unit = array1 / mag;

		return unit;
	}*/

	/*==============================================================================
	* Function: rotate::MAngleBetween
	*
	* Purpose: Function calculates the angle between a corresponding list of vectors
	*
	* Inputs: array1 -- a 3 x n list of vectors
	*		  array2 -- a 3 x n list of vectors
	*
	*
	* Return: Returns a vector holding the angle between corresponding vectors
	*
	* Notes: None
	*=============================================================================*/
	arma::mat MAngleBetween(arma::mat array1, arma::mat array2) {

		double dSmall = 1 * std::pow(10, -6);
		arma::mat u1, u2;
		arma::mat dotP, theta, angle;

		u1 = MUnit(array1);
		u2 = MUnit(array2);

		dotP = MDot(u1, u2);

		arma::uvec indexAboveOne  = arma::find(dotP > 1 && dotP <= 1. + dSmall);
		arma::uvec indexBelowZero = arma::find(dotP < 0 && dotP <= 0. - dSmall);

		if (indexAboveOne.n_elem > 0) {
			dotP(indexAboveOne) = arma::ones(indexAboveOne.n_elem);
		}

		if (indexBelowZero.n_elem > 0) {
			dotP(indexBelowZero) = arma::zeros(indexBelowZero.n_elem);
		}


		theta = arma::acos(dotP);
		angle = theta * (180 / M_PI);

		return angle.t();
	}

	/*double MAngleBetween(arma::vec array1, arma::vec array2) {

		arma::vec u1, u2;
		double dotP, theta, angle;

		u1 = MUnit(array1);
		u2 = MUnit(array2);

		dotP = MDot(u1, u2);
		theta = std::acos(dotP);
		angle = theta * (180 / M_PI);

		return angle;
	}*/
}
