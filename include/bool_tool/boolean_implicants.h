#ifndef BOOLEAN_IMPLICANTS_H
#define BOOLEAN_IMPLICANTS_H

#include <bool_tool/boolean_minterms.h>

struct boolean_implicants {
	struct boolean_implicant {
		uint64_t value;
		uint64_t mask;
	} *data;
	size_t length;
	size_t capacity;
};

void boolean_implicants_drop(struct boolean_implicants *implicants);
struct boolean_implicants boolean_implicants_new(void);
struct boolean_implicants boolean_implicants_from_minterms(
	const struct boolean_minterms *minterms,
	const struct boolean_variables *variables
);
void boolean_implicants_minimalize(
	struct boolean_implicants *implicants,
	const struct boolean_minterms *minterms
);

#endif
