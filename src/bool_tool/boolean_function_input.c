#include <bool_tool/boolean_function_input.h>

#include <gtk/gtk.h>

struct _BooleanFunctionInput {
	GtkBox parent;
	enum boolean_function_representation representation;
	GtkStack *stack;
	GtkEntry *expression_entry;
	GtkEntry *minterms_entry;
	GtkEntry *minterms_variables_entry;
};

G_DEFINE_TYPE(BooleanFunctionInput, boolean_function_input, GTK_TYPE_BOX)

enum {
	CHANGED,
	LAST_SIGNAL,
};

static guint signals[LAST_SIGNAL] = { 0 };

static void boolean_function_input_dispose(GObject *gobject) {
	gtk_widget_dispose_template(GTK_WIDGET(gobject), BOOLEAN_FUNCTION_INPUT_TYPE);

	G_OBJECT_CLASS(boolean_function_input_parent_class)->dispose(gobject);
}

static void boolean_function_input_class_init(BooleanFunctionInputClass *class) {
	G_OBJECT_CLASS(class)->dispose = boolean_function_input_dispose;

	GObjectClass *gobject_class = G_OBJECT_CLASS(class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/com/github/TarekSaeed0/bool_tool/ui/boolean_function_input.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, BooleanFunctionInput, stack);
	gtk_widget_class_bind_template_child(widget_class, BooleanFunctionInput, expression_entry);
	gtk_widget_class_bind_template_child(widget_class, BooleanFunctionInput, minterms_entry);
	gtk_widget_class_bind_template_child(
		widget_class,
		BooleanFunctionInput,
		minterms_variables_entry
	);

	signals[CHANGED] = g_signal_new(
		"changed",
		G_OBJECT_CLASS_TYPE(gobject_class),
		G_SIGNAL_RUN_LAST,
		0,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		0
	);
}

static void boolean_function_input_init(BooleanFunctionInput *self) {
	gtk_widget_init_template(GTK_WIDGET(self));
}

struct boolean_function boolean_function_input_get_function(const BooleanFunctionInput *self) {
	struct boolean_function function;
	switch (self->representation) {
		case boolean_function_representation_expression: {
			const char *expression_string =
				gtk_editable_get_text(GTK_EDITABLE(self->expression_entry));

			struct boolean_expression expression =
				boolean_expression_from_string(expression_string);

			function = boolean_function_new_expression(expression);
		} break;
		case boolean_function_representation_minterms: {
			const char *minterms_string = gtk_editable_get_text(GTK_EDITABLE(self->minterms_entry));
			const char *variables_string =
				gtk_editable_get_text(GTK_EDITABLE(self->minterms_variables_entry));

			struct boolean_minterms minterms = boolean_minterms_from_string(minterms_string);
			struct boolean_variables variables = boolean_variables_from_string(variables_string);

			function = boolean_function_new_minterms(minterms, variables);
		} break;
	}

	return function;
}

G_MODULE_EXPORT void dropdown_selected_changed(GtkDropDown *self, gpointer user_data) {
	(void)user_data;

	BooleanFunctionInput *input = BOOLEAN_FUNCTION_INPUT(
		gtk_widget_get_ancestor(GTK_WIDGET(self), BOOLEAN_FUNCTION_INPUT_TYPE)
	);

	guint selected = gtk_drop_down_get_selected(self);

	switch (selected) {
		case 0: {
			gtk_editable_delete_text(GTK_EDITABLE(input->expression_entry), 0, -1);
			gtk_stack_set_visible_child_name(input->stack, "expression");
			input->representation = boolean_function_representation_expression;
		} break;
		case 1: {
			gtk_editable_delete_text(GTK_EDITABLE(input->minterms_entry), 0, -1);
			gtk_editable_delete_text(GTK_EDITABLE(input->minterms_variables_entry), 0, -1);
			gtk_stack_set_visible_child_name(input->stack, "minterms");
			input->representation = boolean_function_representation_minterms;
		} break;
		default:;
	}

	g_signal_emit(input, signals[CHANGED], 0);
}
