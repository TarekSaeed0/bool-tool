#include <bool_tool/boolean_function.h>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct boolean_variables boolean_variables_clone(const struct boolean_variables *variables) {
	assert(variables != NULL);

	struct boolean_variables clone = {
		.data = malloc(variables->length * sizeof(*variables->data)),
		.length = variables->length,
	};

	memcpy(clone.data, variables->data, variables->length * sizeof(*variables->data));

	return clone;
}

void boolean_variables_drop(struct boolean_variables *variables) {
	assert(variables != NULL);

	free(variables->data);
}

struct boolean_variables boolean_variables_from_string(const char *string) {
	size_t length = 1;
	for (size_t i = 0; string[i] != '\0'; i++) {
		if (string[i] == ',') {
			length++;
		}
	}

	struct boolean_variables variables = {
		.data = malloc(length * sizeof(*variables.data)),
		.length = 0,
	};
	assert(variables.data != NULL);

	for (const char *i = string; *i != '\0'; i++) {
		while (isspace(*i)) {
			i++;
		}
		if (*i == '\0') {
			break;
		}
		assert(variables.length < length);
		variables.data[variables.length++] = *i;

		i = strchr(i + 1, ',');
		if (i == NULL) {
			break;
		}
	}

	return variables;
}

static int boolean_variables_to_string_(
	char *string,
	size_t maximum_length,
	const struct boolean_variables *variables
) {
	assert(variables != NULL);

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

	for (size_t i = 0; i < variables->length; i++) {
		if (i != 0) {
			print(snprintf, ", ");
		}
		print(snprintf, "%c", variables->data[i]);
	}

	return total_length;
#undef print
}
char *boolean_variables_to_string(const struct boolean_variables *variables) {
	assert(variables != NULL);

	int length = boolean_variables_to_string_(NULL, 0, variables);
	if (length < 0) {
		return NULL;
	}

	char *string = malloc((size_t)length + 1);
	if (string == NULL) {
		return NULL;
	}

	if (boolean_variables_to_string_(string, (size_t)length + 1, variables) < 0) {
		free(string);
		return NULL;
	}

	return string;
}

void boolean_variables_from_expression_(
	const struct boolean_expression *expression,
	struct boolean_environment *environment
) {
	assert(expression != NULL && environment != NULL);

	switch (expression->type) {
		case boolean_expression_type_constant: break;
		case boolean_expression_type_variable: {
			boolean_environment_set_variable(environment, expression->variable.name, true);
		} break;
		case boolean_expression_type_operation: {
			size_t arity = boolean_operation_type_arity(expression->operation.type);
			for (size_t i = 0; i < arity; i++) {
				boolean_variables_from_expression_(&expression->operation.operands[i], environment);
			}
		} break;
	}
}
struct boolean_variables boolean_variables_from_expression(
	const struct boolean_expression *expression
) {
	assert(expression != NULL);

	struct boolean_environment environment = boolean_environment_new();
	boolean_variables_from_expression_(expression, &environment);

#define VARIABLES_COUNT (('z' - 'a' + 1) + ('Z' - 'A' + 1))
#define VARIABLE_NAME_FROM_INDEX(index)                                                            \
	(char)((index) < ('z' - 'a' + 1) ? 'a' + (index) : 'A' + ((index) - ('z' - 'a' + 1)))

	struct boolean_variables variables = {
		.data = malloc(VARIABLES_COUNT),
		.length = 0,
	};
	for (size_t i = 0; i < VARIABLES_COUNT; i++) {
		if ((environment.variables >> i) & 1U) {
			variables.data[variables.length++] = VARIABLE_NAME_FROM_INDEX(i);
		}
	}

	return variables;
}
