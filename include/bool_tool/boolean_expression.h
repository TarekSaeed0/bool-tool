#ifndef BOOLEAN_EXPRESSION_H
#define BOOLEAN_EXPRESSION_H

#include <bool_tool/boolean_environment.h>
#include <stddef.h>

struct boolean_expression {
	enum boolean_expression_type {
		boolean_expression_type_constant,
		boolean_expression_type_variable,
		boolean_expression_type_operation,
	} type;
	union {
		struct boolean_constant {
			bool value;
		} constant;
		struct boolean_variable {
			char name;
		} variable;
		struct boolean_operation {
			enum boolean_operation_type {
				boolean_operation_type_conjunction,
				boolean_operation_type_disjunction,
				boolean_operation_type_negation,
			} type;
			struct boolean_expression *operands;
		} operation;
	};
};

size_t boolean_operation_type_arity(enum boolean_operation_type type);
size_t boolean_operation_type_precedence(enum boolean_operation_type type);

void boolean_expression_drop(struct boolean_expression *expression);
struct boolean_expression boolean_expression_new_constant(bool value);
struct boolean_expression boolean_expression_new_variable(char name);
struct boolean_expression boolean_expression_new_operation(enum boolean_operation_type type, ...);
struct boolean_expression boolean_expression_clone(const struct boolean_expression *expression);
struct boolean_expression boolean_expression_from_string(const char *string);
char *boolean_expression_to_string(const struct boolean_expression *expression);
struct boolean_minterms;
struct boolean_variables;
struct boolean_expression boolean_expression_from_minterms(
	const struct boolean_minterms *minterms,
	const struct boolean_variables *variables
);
struct boolean_implicants;
struct boolean_expression boolean_expression_from_implicants(
	const struct boolean_implicants *implicants,
	const struct boolean_variables *variables
);
bool boolean_expression_evaluate(
	const struct boolean_expression *expression,
	const struct boolean_environment *environment
);

#endif
