#ifndef _PTR_COMP_H_
#define _PTR_COMP_H_

template< typename T> struct ptr_comp {
	bool operator()(const T * a, const T * b) const {
		return ((*a) < (*b));
	}
};

#endif
