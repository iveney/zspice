#ifndef __UTIL_H__
#define __UTIL_H__

#include <vector>
using std::vector;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)
#define report_exit(a) _report_exit(AT,a)

void _report_exit(const char *location, const char *msg);
double ** new_square_matrix(int n);
void delete_matrix(double **,int);
void output_matrix(double **,int);

// given a vector, copy its element to a basic array
template<class T>
void vector_to_array(vector<T> v, T * arr){
	copy(v.begin(), v.end(), arr);
}

#endif
