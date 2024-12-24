#ifndef BOOLEAN_STATUS_H
#define BOOLEAN_STATUS_H

struct boolean_status {
	struct boolean_status_failure {
		union {
			const char *literal;
			char *formatted;
		};
		enum boolean_status_failure_type {
			boolean_status_failure_type_literal,
			boolean_status_failure_type_formatted,
		} type;
	} failure;
	enum boolean_status_type {
		boolean_status_type_success,
		boolean_status_type_failure,
	} type;
};
struct boolean_status boolean_status_success(void);
struct boolean_status boolean_status_failure(const char *format, ...)
	__attribute__((__format__(__printf__, 1, 2)));
void boolean_status_drop(struct boolean_status *self);
void boolean_status_print(const struct boolean_status *self);

#endif
