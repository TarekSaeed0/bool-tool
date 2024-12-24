#include <bool_tool/boolean_implicants.h>

#include <assert.h>
#include <bool_tool/boolean_variables.h>
#include <stdlib.h>
#include <string.h>

static bool boolean_implicant_combinable(
	struct boolean_implicant implicant_1,
	struct boolean_implicant implicant_2
) {
	// two implicants can possibly be combined if their masks are equal
	if (implicant_1.mask != implicant_2.mask) {
		return false;
	}

	// two implicants with equal masks can be combined if they only differ in one bit
	uint64_t difference = (implicant_1.value ^ implicant_2.value) & implicant_1.mask;
	int ones_count = __builtin_popcountll(difference);

	return ones_count == 1;
}

static struct boolean_implicant boolean_implicant_combine(
	struct boolean_implicant implicant_1,
	struct boolean_implicant implicant_2
) {
	assert(boolean_implicant_combinable(implicant_1, implicant_2));

	return (struct boolean_implicant){
		.value = implicant_1.value,
		.mask = implicant_1.mask & ~(implicant_1.value ^ implicant_2.value),
	};
}

void boolean_implicants_drop(struct boolean_implicants *implicants) {
	assert(implicants != NULL);

	free(implicants->data);
}

struct boolean_implicants boolean_implicants_new(void) {
	return (struct boolean_implicants){
		.data = NULL,
		.length = 0,
		.capacity = 0,
	};
}

void boolean_implicants_add(
	struct boolean_implicants *implicants,
	struct boolean_implicant implicant
) {
	assert(implicants != NULL);

	if (implicants->length == implicants->capacity) {
		if (implicants->capacity == 0) {
			implicants->capacity = 1;
		} else {
			assert(implicants->capacity < SIZE_MAX / 2);
			implicants->capacity *= 2;
		}

		struct boolean_implicant *data =
			realloc(implicants->data, implicants->capacity * sizeof(*implicants->data));
		assert(data != NULL);

		implicants->data = data;
	}

	implicants->data[implicants->length++] = implicant;
}

struct table {
	struct group {
		struct term {
			struct boolean_implicant implicant;
			bool combined;
		} *terms;
		size_t terms_count;
		size_t terms_capacity;
	} *groups;
	size_t groups_count;
};
struct table table_new(size_t groups_count) {
	struct table table = {
		.groups = malloc(groups_count * sizeof(*table.groups)),
		.groups_count = groups_count,
	};

	for (size_t j = 0; j < groups_count; j++) {
		table.groups[j] = (struct group){
			.terms = NULL,
			.terms_count = 0,
			.terms_capacity = 0,
		};
	}

	return table;
}
void table_drop(struct table *table) {
	assert(table != NULL);

	for (size_t i = 0; i < table->groups_count; i++) {
		free(table->groups[i].terms);
	}
	free(table->groups);
}
void table_add_implicant(struct table *table, struct boolean_implicant implicant) {
	assert(table != NULL);

	int ones_count = __builtin_popcountll((implicant.value & implicant.mask));
	assert(0 <= ones_count && (size_t)ones_count < table->groups_count);

	// if implicant is already in group, then don't add it
	bool is_duplicate = false;
	for (size_t i = 0; i < table->groups[ones_count].terms_count; i++) {
		if (table->groups[ones_count].terms[i].implicant.mask == implicant.mask &&
			table->groups[ones_count].terms[i].implicant.value == implicant.value) {
			is_duplicate = true;
			break;
		}
	}
	if (!is_duplicate) {
		// if there isn't enough space for a new term, reallocate.
		if (table->groups[ones_count].terms_count == table->groups[ones_count].terms_capacity) {
			if (table->groups[ones_count].terms_capacity == 0) {
				table->groups[ones_count].terms_capacity = 1;
			} else {
				assert(table->groups[ones_count].terms_capacity < SIZE_MAX / 2);
				table->groups[ones_count].terms_capacity *= 2;
			}
			table->groups[ones_count].terms = realloc(
				table->groups[ones_count].terms,
				table->groups[ones_count].terms_capacity * sizeof(*table->groups[ones_count].terms)
			);
			assert(table->groups[ones_count].terms != NULL);
		}
		table->groups[ones_count].terms[table->groups[ones_count].terms_count++] = (struct term){
			.implicant = implicant,
			.combined = false,
		};
	}
}
struct boolean_implicants boolean_implicants_from_minterms(
	const struct boolean_minterms *minterms,
	const struct boolean_variables *variables
) {
	assert(minterms != NULL && variables != NULL);

	struct table input_table = table_new(variables->length + 1);
	struct table output_table = table_new(variables->length + 1);

	for (size_t i = 0; i < minterms->length; i++) {
		table_add_implicant(
			&input_table,
			(struct boolean_implicant){
				.value = minterms->data[i],
				.mask = (UINT64_C(1) << variables->length) - 1U,
			}
		);
	}

	struct boolean_implicants prime_implicants = boolean_implicants_new();

	bool minimized = true;
	do {
		minimized = true;

		for (size_t i = 0; i < input_table.groups_count; i++) {
			for (size_t j = 0; j < input_table.groups[i].terms_count; j++) {
				if (i != input_table.groups_count - 1) {
					for (size_t k = 0; k < input_table.groups[i + 1].terms_count; k++) {
						// two implicants can be combined if their masks are equal
						if (boolean_implicant_combinable(
								input_table.groups[i].terms[j].implicant,
								input_table.groups[i + 1].terms[k].implicant
							)) {

							input_table.groups[i].terms[j].combined = true;
							input_table.groups[i + 1].terms[k].combined = true;

							minimized = false;

							table_add_implicant(
								&output_table,
								boolean_implicant_combine(
									input_table.groups[i].terms[j].implicant,
									input_table.groups[i + 1].terms[k].implicant
								)
							);
						}
					}
				}

				if (!input_table.groups[i].terms[j].combined) {
					boolean_implicants_add(
						&prime_implicants,
						input_table.groups[i].terms[j].implicant
					);
				}
			}
		}

		struct table table = input_table;
		input_table = output_table;
		output_table = table;
		for (size_t i = 0; i < output_table.groups_count; i++) {
			output_table.groups[i].terms_count = 0;
		}
	} while (!minimized);

	table_drop(&input_table);
	table_drop(&output_table);

	return prime_implicants;
}

void boolean_implicants_minimalize(
	struct boolean_implicants *implicants,
	const struct boolean_minterms *minterms
) {
	assert(implicants != NULL && minterms != NULL);

	struct {
		size_t *data;
		size_t length;
	} *factors = malloc(minterms->length * sizeof(*factors));
	assert(factors != NULL);

	size_t *frequencies = malloc(implicants->length * sizeof(*frequencies));
	memset(frequencies, 0, implicants->length * sizeof(*frequencies));

	for (size_t i = 0; i < minterms->length; i++) {
		factors[i].data = malloc(implicants->length * sizeof(*factors[i].data));
		assert(factors[i].data != NULL);
		factors[i].length = 0;

		for (size_t j = 0; j < implicants->length; j++) {
			if (((implicants->data[j].value ^ minterms->data[i]) & implicants->data[j].mask) == 0) {
				factors[i].data[factors[i].length++] = j;
				frequencies[j]++;
			}
		}
	}

	bool *minimal = malloc(implicants->length * sizeof(*minimal));
	memset(minimal, false, implicants->length * sizeof(*minimal));
	for (size_t i = 0; i < minterms->length; i++) {
		bool absorbed = false;
		for (size_t j = 0; j < factors[i].length; j++) {
			if (minimal[factors[i].data[j]]) {
				absorbed = true;
				break;
			}
		}
		if (!absorbed) {
			size_t most_frequent = factors[i].data[0];
			for (size_t j = 1; j < factors[i].length; j++) {
				if (frequencies[factors[i].data[j]] > frequencies[most_frequent]) {
					most_frequent = factors[i].data[j];
				}
			}
			minimal[most_frequent] = true;
		}

		for (size_t j = 0; j < factors[i].length; j++) {
			frequencies[factors[i].data[j]]--;
		}
	}

	free(frequencies);

	for (size_t i = 0; i < minterms->length; i++) {
		free(factors[i].data);
	}
	free(factors);

	for (ptrdiff_t i = (ptrdiff_t)implicants->length - 1; i >= 0; i--) {
		if (!minimal[i]) {
			memmove(
				&implicants->data[i],
				&implicants->data[i + 1],
				(implicants->length - (size_t)i - 1) * sizeof(*implicants->data)
			);
			implicants->length--;
		}
	}

	free(minimal);
}
