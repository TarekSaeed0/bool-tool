#ifndef BOOLEAN_VARIABLES_H
#define BOOLEAN_VARIABLES_H

#include <bool_tool/boolean_expression.h>

struct boolean_variables {
	char *data;
	size_t length;
};

void boolean_variables_drop(struct boolean_variables *variables);
struct boolean_variables boolean_variables_clone(const struct boolean_variables *variables);
struct boolean_variables boolean_variables_from_string(const char *string);
char *boolean_variables_to_string(const struct boolean_variables *variables);
struct boolean_variables boolean_variables_from_expression(
	const struct boolean_expression *expression
);

#endif
