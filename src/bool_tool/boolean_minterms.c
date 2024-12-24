#include <bool_tool/boolean_minterms.h>

#include <assert.h>
#include <bool_tool/boolean_variables.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct boolean_minterms boolean_minterms_clone(const struct boolean_minterms *minterms) {
	assert(minterms != NULL);

	struct boolean_minterms clone = {
		.data = malloc(minterms->length * sizeof(*minterms->data)),
		.length = minterms->length,
	};
	assert(clone.data != NULL);

	memcpy(clone.data, minterms->data, minterms->length * sizeof(*minterms->data));

	return clone;
}

void boolean_minterms_drop(struct boolean_minterms *minterms) {
	assert(minterms != NULL);

	free(minterms->data);
}

struct boolean_minterms boolean_minterms_from_string(const char *string) {
	size_t length = 1;
	for (size_t i = 0; string[i] != '\0'; i++) {
		if (string[i] == ',') {
			length++;
		}
	}

	struct boolean_minterms minterms = {
		.data = malloc(length * sizeof(*minterms.data)),
		.length = 0,
	};
	assert(minterms.data != NULL);

	for (const char *i = string; *i != '\0'; i++) {
		char *end = NULL;
		assert(minterms.length < length);
		minterms.data[minterms.length++] = (uint64_t)strtoumax(i, &end, 10);

		if (end == i) {
			(void)fprintf(stderr, "Error: failed to parse minterm from \"%s\"\n", i);
		}

		if (errno == ERANGE) {
			(void)fprintf(stderr, "Warning: minterm parsed from \"%s\" is out of range\n", i);
		}

		i = strchr(end, ',');
		if (i == NULL) {
			break;
		}
	}

	return minterms;
}

static int boolean_minterms_to_string_(
	char *string,
	size_t maximum_length,
	const struct boolean_minterms *minterms
) {
	assert(minterms != NULL);

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

	for (size_t i = 0; i < minterms->length; i++) {
		if (i != 0) {
			print(snprintf, ", ");
		}
		print(snprintf, "%" PRIu64, minterms->data[i]);
	}

	return total_length;
#undef print
}
char *boolean_minterms_to_string(const struct boolean_minterms *minterms) {
	assert(minterms != NULL);

	int length = boolean_minterms_to_string_(NULL, 0, minterms);
	if (length < 0) {
		return NULL;
	}

	char *string = malloc((size_t)length + 1);
	if (string == NULL) {
		return NULL;
	}

	if (boolean_minterms_to_string_(string, (size_t)length + 1, minterms) < 0) {
		free(string);
		return NULL;
	}

	return string;
}

struct boolean_minterms boolean_minterms_from_expression(const struct boolean_expression *expression
) {
	assert(expression != NULL);

	struct boolean_variables variables = boolean_variables_from_expression(expression);

	struct boolean_minterms minterms = {
		.data = malloc((1U << variables.length) * sizeof(*minterms.data)),
		.length = 0,
	};

	for (uint64_t i = 0; i < (1U << variables.length); i++) {
		struct boolean_environment environment = boolean_environment_new();
		for (size_t j = 0; j < variables.length; j++) {
			boolean_environment_set_variable(
				&environment,
				variables.data[variables.length - j - 1],
				(i >> j) & 1U
			);
		}

		if (boolean_expression_evaluate(expression, &environment)) {
			minterms.data[minterms.length++] = i;
		}
	}

	boolean_variables_drop(&variables);

	return minterms;
}
