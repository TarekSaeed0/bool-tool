#include <bool_tool/boolean_function.h>

#include <assert.h>

struct boolean_function boolean_function_clone(const struct boolean_function *function) {
	assert(function != NULL);

	switch (function->representation) {
		case boolean_function_representation_expression: {
			return boolean_function_new_expression(boolean_expression_clone(&function->expression));
		} break;
		case boolean_function_representation_minterms: {
			return boolean_function_new_minterms(
				boolean_minterms_clone(&function->minterms),
				boolean_variables_clone(&function->variables)
			);
		} break;
	}
}

struct boolean_function boolean_function_new_expression(struct boolean_expression expression) {
	return (struct boolean_function){
		.representation = boolean_function_representation_expression,
		.expression = expression,
	};
}
struct boolean_function boolean_function_new_minterms(
	struct boolean_minterms minterms,
	struct boolean_variables variables
) {
	return (struct boolean_function){
		.representation = boolean_function_representation_minterms,
		.minterms = minterms,
		.variables = variables,
	};
}

void boolean_function_drop(struct boolean_function *function) {
	assert(function != NULL);

	switch (function->representation) {
		case boolean_function_representation_expression: {
			boolean_expression_drop(&function->expression);
		} break;
		case boolean_function_representation_minterms: {
			boolean_minterms_drop(&function->minterms);
			boolean_variables_drop(&function->variables);
		} break;
	}
}
