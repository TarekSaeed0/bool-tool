#ifndef BOOLEAN_ENVIRONMENT_H
#define BOOLEAN_ENVIRONMENT_H

#include <stdbool.h>
#include <stdint.h>

struct boolean_environment {
	uint64_t variables;
};

struct boolean_environment boolean_environment_new(void);
bool boolean_environment_get_variable(const struct boolean_environment *environment, char name);
void boolean_environment_set_variable(
	struct boolean_environment *environment,
	char name,
	bool value
);

#endif
