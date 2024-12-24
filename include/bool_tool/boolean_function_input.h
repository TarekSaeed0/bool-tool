#ifndef BOOLEAN_FUNCTION_INPUT_H
#define BOOLEAN_FUNCTION_INPUT_H

#include <bool_tool/boolean_function.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BOOLEAN_FUNCTION_INPUT_TYPE boolean_function_input_get_type()
G_DECLARE_FINAL_TYPE(BooleanFunctionInput, boolean_function_input, BOOLEAN_FUNCTION, INPUT, GtkBox)

struct boolean_function boolean_function_input_get_function(const BooleanFunctionInput *self);

G_END_DECLS

#endif
