#include <bool_tool/boolean_environment.h>

#include <assert.h>
#include <ctype.h>
#include <stddef.h>

#define VARIABLE_NAME_TO_INDEX(name)                                                               \
	(islower(name) ? (size_t)((name) - 'a') : (size_t)((name) - 'A' + ('z' - 'a' + 1)))

struct boolean_environment boolean_environment_new(void) {
	return (struct boolean_environment){
		.variables = 0,
	};
}

bool boolean_environment_get_variable(const struct boolean_environment *environment, char name) {
	assert(environment != NULL && isalpha((unsigned char)name));

	return (environment->variables >> VARIABLE_NAME_TO_INDEX(name)) & 1U;
}

void boolean_environment_set_variable(
	struct boolean_environment *environment,
	char name,
	bool value
) {
	assert(environment != NULL && isalpha((unsigned char)name));

	environment->variables &= ~(1U << VARIABLE_NAME_TO_INDEX(name));
	environment->variables |= ((unsigned)value << VARIABLE_NAME_TO_INDEX(name));
}
