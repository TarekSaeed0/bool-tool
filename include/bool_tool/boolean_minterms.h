#ifndef BOOLEAN_MINTERMS_H
#define BOOLEAN_MINTERMS_H

#include <bool_tool/boolean_expression.h>

struct boolean_minterms {
	uint64_t *data;
	size_t length;
};

void boolean_minterms_drop(struct boolean_minterms *minterms);
struct boolean_minterms boolean_minterms_clone(const struct boolean_minterms *minterms);
struct boolean_minterms boolean_minterms_from_string(const char *string);
char *boolean_minterms_to_string(const struct boolean_minterms *minterms);
struct boolean_minterms boolean_minterms_from_expression(const struct boolean_expression *expression
);

#endif
