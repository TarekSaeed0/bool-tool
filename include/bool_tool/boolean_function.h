#ifndef BOOLEAN_FUNCTION_H
#define BOOLEAN_FUNCTION_H

#include <bool_tool/boolean_expression.h>
#include <bool_tool/boolean_minterms.h>
#include <bool_tool/boolean_variables.h>

struct boolean_function {
	enum boolean_function_representation {
		boolean_function_representation_expression,
		boolean_function_representation_minterms,
	} representation;
	union {
		struct boolean_expression expression;
		struct {
			struct boolean_minterms minterms;
			struct boolean_variables variables;
		};
	};
};

void boolean_function_drop(struct boolean_function *function);
struct boolean_function boolean_function_new_expression(struct boolean_expression expression);
struct boolean_function boolean_function_new_minterms(
	struct boolean_minterms minterms,
	struct boolean_variables variables
);
struct boolean_function boolean_function_clone(const struct boolean_function *function);

#endif
