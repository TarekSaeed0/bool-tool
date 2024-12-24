#include <bool_tool/boolean_expression.h>

#include <assert.h>
#include <bool_tool/boolean_implicants.h>
#include <bool_tool/boolean_minterms.h>
#include <bool_tool/boolean_variables.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t boolean_operation_type_arity(enum boolean_operation_type type) {
	return 1 + (type <= boolean_operation_type_disjunction);
}

size_t boolean_operation_type_precedence(enum boolean_operation_type type) {
	switch (type) {
		case boolean_operation_type_disjunction: return 0;
		case boolean_operation_type_conjunction: return 1;
		case boolean_operation_type_negation: return 2;
		default: assert(false);
	}
}

void boolean_expression_drop(struct boolean_expression *expression) {
	assert(expression != NULL);

	switch (expression->type) {
		case boolean_expression_type_constant:
		case boolean_expression_type_variable: break;
		case boolean_expression_type_operation: {
			size_t arity = boolean_operation_type_arity(expression->operation.type);
			for (size_t i = 0; i < arity; i++) {
				boolean_expression_drop(&expression->operation.operands[i]);
			}
			free(expression->operation.operands);
		} break;
	}
}

struct boolean_expression boolean_expression_new_constant(bool value) {
	return (struct boolean_expression){
		.type = boolean_expression_type_constant,
		.constant = { .value = value },
	};
}
struct boolean_expression boolean_expression_new_variable(char name) {
	assert(isalpha((unsigned char)name));

	return (struct boolean_expression){
		.type = boolean_expression_type_variable,
		.variable = { .name = name },
	};
}
struct boolean_expression boolean_expression_new_operation(enum boolean_operation_type type, ...) {
	va_list arguments;
	va_start(arguments, type);

	size_t arity = boolean_operation_type_arity(type);

	struct boolean_expression *operands = malloc(arity * sizeof(*operands));
	for (size_t i = 0; i < arity; i++) {
		operands[i] = va_arg(arguments, struct boolean_expression);
	}

	va_end(arguments);

	return (struct boolean_expression){
		.type = boolean_expression_type_operation,
		.operation = { .type = type, .operands = operands },
	};
}

struct boolean_expression boolean_expression_clone(const struct boolean_expression *expression) {
	assert(expression != NULL);

	switch (expression->type) {
		case boolean_expression_type_constant:
		case boolean_expression_type_variable: return *expression;
		case boolean_expression_type_operation: {
			size_t arity = boolean_operation_type_arity(expression->operation.type);
			struct boolean_expression *operands = malloc(arity * sizeof(*operands));
			for (size_t i = 0; i < arity; i++) {
				operands[i] = boolean_expression_clone(&expression->operation.operands[i]);
			}
			return (struct boolean_expression){
				.type = boolean_expression_type_operation,
				.operation = { .type = expression->operation.type, .operands = operands },
			};
		} break;
	}

	return (struct boolean_expression){ 0 };
}

static struct boolean_expression boolean_expression_from_string_expression(const char **string);
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
static struct boolean_expression boolean_expression_from_string_atom(const char **string) {
	assert(string != NULL && *string != NULL);

	struct boolean_expression atom;

	while (isspace((unsigned char)**string)) {
		++*string;
	}

	if (**string == '(') {
		++*string;

		atom = boolean_expression_from_string_expression(string);

		while (isspace((unsigned char)**string)) {
			++*string;
		}

		if (**string == ')') {
			++*string;
		} else {
			(void)fprintf(stderr, "Warning: unclosed parentheses \"%s\"\n", *string);
		}
	} else if (isalpha((unsigned char)**string)) {
		char name = **string;

		++*string;

		atom = boolean_expression_new_variable(name);
	} else {
		char *end = NULL;
		long value = strtol(*string, &end, 10);
		if (end == *string) {
			(void)fprintf(stderr, "Error: failed to parse constant from \"%s\"\n", *string);
			value = false;
		}

		if (errno == ERANGE) {
			(void
			)fprintf(stderr, "Warning: constant parsed from \"%s\" is out of range\n", *string);
		}

		if (value != 0 && value != 1) {
			(void)fprintf(
				stderr,
				"Warning: non-zero constant parsed from \"%s\" will be implicitly converted into a "
				"1\n",
				*string
			);
		}

		*string = end;

		atom = boolean_expression_new_constant(value);
	}

	return atom;
}
static struct boolean_expression boolean_expression_from_string_primary(const char **string) {
	assert(string != NULL && *string != NULL);

	struct boolean_expression primary;

	while (isspace((unsigned char)**string)) {
		++*string;
	}

	if (**string == '!') {
		++*string;

		primary = boolean_expression_new_operation(
			boolean_operation_type_negation,
			boolean_expression_from_string_primary(string)
		);
	} else {
		primary = boolean_expression_from_string_atom(string);
	}

	while (isspace((unsigned char)**string)) {
		++*string;
	}

	while (1) {
		while (isspace((unsigned char)**string)) {
			++*string;
		}

		if (**string == '\'') {
			++*string;
			primary = boolean_expression_new_operation(boolean_operation_type_negation, primary);
		} else {
			break;
		}
	}

	return primary;
}
static struct boolean_expression boolean_expression_from_string_factor(const char **string) {
	assert(string != NULL && *string != NULL);

	struct boolean_expression factor = boolean_expression_from_string_primary(string);

	while (1) {
		while (isspace((unsigned char)**string)) {
			++*string;
		}

		if (**string == '!' || **string == '(' || isalpha((unsigned char)**string)) {
			factor = boolean_expression_new_operation(
				boolean_operation_type_conjunction,
				factor,
				boolean_expression_from_string_primary(string)
			);
		} else {
			break;
		}
	}

	return factor;
}
static struct boolean_expression boolean_expression_from_string_term(const char **string) {
	assert(string != NULL && *string != NULL);

	struct boolean_expression expression = boolean_expression_from_string_factor(string);

	while (1) {
		while (isspace((unsigned char)**string)) {
			++*string;
		}

		switch (**string) {
			case '&':
			case '*': {
				++*string;

				expression = boolean_expression_new_operation(
					boolean_operation_type_conjunction,
					expression,
					boolean_expression_from_string_factor(string)
				);
			} break;
			default: return expression;
		}
	}
}
static struct boolean_expression boolean_expression_from_string_expression(const char **string) {
	assert(string != NULL && *string != NULL);

	struct boolean_expression expression = boolean_expression_from_string_term(string);

	while (1) {
		while (isspace((unsigned char)**string)) {
			++*string;
		}

		switch (**string) {
			case '|':
			case '+': {
				++*string;

				expression = boolean_expression_new_operation(
					boolean_operation_type_disjunction,
					expression,
					boolean_expression_from_string_term(string)
				);
			} break;
			default: return expression;
		}
	}
}

struct boolean_expression boolean_expression_from_string(const char *string) {
	assert(string != NULL);

	struct boolean_expression expression = boolean_expression_from_string_expression(&string);

	while (isspace((unsigned char)*string)) {
		++string;
	}

	if (*string != '\0') {
		(void)fprintf(stderr, "Warning: trailing characters \"%s\" after expression\n", string);
	}

	return expression;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
static int boolean_expression_to_string_(
	char *string,
	size_t maximum_length,
	const struct boolean_expression *expression
) {
	assert(expression != NULL);

	int total_length = 0;
#define print(function, ...)                                                                       \
	do {                                                                                           \
		int length = function(string, maximum_length, __VA_ARGS__);                                \
		if (length < 0) {                                                                          \
			return length;                                                                         \
		}                                                                                          \
		if ((size_t)length >= maximum_length) {                                                    \
			if (string != NULL) {                                                                  \
				string += maximum_length;                                                          \
			}                                                                                      \
			maximum_length = 0;                                                                    \
		} else {                                                                                   \
			if (string != NULL) {                                                                  \
				string += length;                                                                  \
			}                                                                                      \
			maximum_length -= (size_t)length;                                                      \
		}                                                                                          \
		total_length += length;                                                                    \
	} while (0)

	switch (expression->type) {
		case boolean_expression_type_constant:
			print(snprintf, "%d", expression->constant.value);
			break;
		case boolean_expression_type_variable:
			print(snprintf, "%c", expression->variable.name);
			break;
		case boolean_expression_type_operation: {
			switch (expression->operation.type) {
				case boolean_operation_type_conjunction:
				case boolean_operation_type_disjunction: {
					if (expression->operation.operands[0].type ==
							boolean_expression_type_operation &&
						(boolean_operation_type_precedence(
							 expression->operation.operands[0].operation.type
						 ) < boolean_operation_type_precedence(expression->operation.type))) {
						print(snprintf, "(");
						print(boolean_expression_to_string_, &expression->operation.operands[0]);
						print(snprintf, ")");
					} else {
						print(boolean_expression_to_string_, &expression->operation.operands[0]);
					}

					switch (expression->operation.type) {
						case boolean_operation_type_conjunction: {
							if (expression->operation.operands[0].type ==
									boolean_expression_type_constant ||
								expression->operation.operands[1].type ==
									boolean_expression_type_constant) {
								print(snprintf, " * ");
							}
						} break;
						case boolean_operation_type_disjunction: print(snprintf, " + "); break;
						// we have already checked the operation's type before
						default: __builtin_unreachable();
					}

					if (expression->operation.operands[1].type ==
							boolean_expression_type_operation &&
						(boolean_operation_type_precedence(
							 expression->operation.operands[1].operation.type
						 ) <= boolean_operation_type_precedence(expression->operation.type))) {
						print(snprintf, "(");
						print(boolean_expression_to_string_, &expression->operation.operands[1]);
						print(snprintf, ")");
					} else {
						print(boolean_expression_to_string_, &expression->operation.operands[1]);
					}
				} break;
				case boolean_operation_type_negation: {
					if (expression->operation.operands[0].type ==
							boolean_expression_type_operation &&
						(boolean_operation_type_precedence(
							 expression->operation.operands[0].operation.type
						 ) < boolean_operation_type_precedence(expression->operation.type))) {
						print(snprintf, "(");
						print(boolean_expression_to_string_, &expression->operation.operands[0]);
						print(snprintf, ")");
					} else {
						print(boolean_expression_to_string_, &expression->operation.operands[0]);
					}
					print(snprintf, "'");

				} break;
			}
		} break;
	}

	return total_length;
#undef print
}
char *boolean_expression_to_string(const struct boolean_expression *expression) {
	assert(expression != NULL);

	int length = boolean_expression_to_string_(NULL, 0, expression);
	if (length < 0) {
		return NULL;
	}

	char *string = malloc((size_t)length + 1);
	if (string == NULL) {
		return NULL;
	}

	if (boolean_expression_to_string_(string, (size_t)length + 1, expression) < 0) {
		free(string);
		return NULL;
	}

	return string;
}

struct boolean_expression boolean_expression_from_minterm(
	uint64_t minterm,
	const struct boolean_variables *variables
) {
	assert(variables != NULL);

	struct boolean_expression expression = boolean_expression_new_variable(variables->data[0]);
	if (((minterm >> (variables->length - 1)) & 1U) == 0) {
		expression = boolean_expression_new_operation(boolean_operation_type_negation, expression);
	}
	for (size_t index = 0; index < variables->length; index++) {
		struct boolean_expression boolean_expression_ =
			boolean_expression_new_variable(variables->data[index]);
		if (((minterm >> (variables->length - index - 1)) & 1U) == 0) {
			boolean_expression_ = boolean_expression_new_operation(
				boolean_operation_type_negation,
				boolean_expression_
			);
		}

		expression = boolean_expression_new_operation(
			boolean_operation_type_conjunction,
			expression,
			boolean_expression_
		);
	}

	return expression;
}
struct boolean_expression boolean_expression_from_minterms(
	const struct boolean_minterms *minterms,
	const struct boolean_variables *variables
) {
	assert(minterms != NULL && variables != NULL);

	if (minterms->length == 0) {
		return boolean_expression_new_constant(false);
	}

	struct boolean_expression expression =
		boolean_expression_from_minterm(minterms->data[0], variables);
	for (size_t i = 1; i < minterms->length; i++) {
		expression = boolean_expression_new_operation(
			boolean_operation_type_disjunction,
			expression,
			boolean_expression_from_minterm(minterms->data[i], variables)
		);
	}

	return expression;
}

struct boolean_expression boolean_expression_from_implicant(
	struct boolean_implicant implicant,
	const struct boolean_variables *variables
) {
	assert(variables != NULL);

	size_t index = 0;
	while (index < variables->length &&
		   ((implicant.mask >> (variables->length - index - 1)) & 1U) == 0) {
		index++;
	}

	if (index >= variables->length) {
		return boolean_expression_new_constant(true);
	}

	struct boolean_expression expression = boolean_expression_new_variable(variables->data[index]);
	if (((implicant.value >> (variables->length - index - 1)) & 1U) == 0) {
		expression = boolean_expression_new_operation(boolean_operation_type_negation, expression);
	}
	for (index++; index < variables->length; index++) {
		if (((implicant.mask >> (variables->length - index - 1)) & 1U) == 0) {
			continue;
		}

		struct boolean_expression boolean_expression_ =
			boolean_expression_new_variable(variables->data[index]);
		if (((implicant.value >> (variables->length - index - 1)) & 1U) == 0) {
			boolean_expression_ = boolean_expression_new_operation(
				boolean_operation_type_negation,
				boolean_expression_
			);
		}

		expression = boolean_expression_new_operation(
			boolean_operation_type_conjunction,
			expression,
			boolean_expression_
		);
	}

	return expression;
}
struct boolean_expression boolean_expression_from_implicants(
	const struct boolean_implicants *implicants,
	const struct boolean_variables *variables
) {
	assert(implicants != NULL && variables != NULL);

	if (implicants->length == 0) {
		return boolean_expression_new_constant(false);
	}

	struct boolean_expression expression =
		boolean_expression_from_implicant(implicants->data[0], variables);
	for (size_t i = 1; i < implicants->length; i++) {
		expression = boolean_expression_new_operation(
			boolean_operation_type_disjunction,
			expression,
			boolean_expression_from_implicant(implicants->data[i], variables)
		);
	}

	return expression;
}

bool boolean_expression_evaluate(
	const struct boolean_expression *expression,
	const struct boolean_environment *environment
) {
	assert(expression != NULL);

	switch (expression->type) {
		case boolean_expression_type_constant: return expression->constant.value;
		case boolean_expression_type_variable: {
			if (environment == NULL) {
				return false;
			}

			return boolean_environment_get_variable(environment, expression->variable.name);
		} break;
		case boolean_expression_type_operation: {
			switch (expression->operation.type) {
				case boolean_operation_type_conjunction: {
					bool value = boolean_expression_evaluate(
						&expression->operation.operands[0],
						environment
					);
					if (!value) {
						return false;
					}
					return boolean_expression_evaluate(
						&expression->operation.operands[1],
						environment
					);
				}
				case boolean_operation_type_disjunction: {
					bool value = boolean_expression_evaluate(
						&expression->operation.operands[0],
						environment
					);
					if (value) {
						return true;
					}
					return boolean_expression_evaluate(
						&expression->operation.operands[1],
						environment
					);
				}
				case boolean_operation_type_negation: {
					return !boolean_expression_evaluate(
						&expression->operation.operands[0],
						environment
					);
				}
			}
		} break;
	}
}
