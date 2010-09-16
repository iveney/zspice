#ifndef __UTIL_H__
#define __UTIL_H__

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)
#define report_exit(a) _report_exit(AT,a)

void _report_exit(const char *location, const char *msg);
double ** new_square_matrix(int n);
void delete_matrix(double **,int);
void output_matrix(double **,int);

#endif
