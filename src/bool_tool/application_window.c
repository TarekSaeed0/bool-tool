#include "bool_tool/boolean_function.h"
#include <bool_tool/application_window.h>

#include <bool_tool/boolean_function_input.h>
#include <bool_tool/boolean_implicants.h>
#include <gtk/gtk.h>

struct _BoolToolApplicationWindow {
	GtkApplicationWindow parent;
	BooleanFunctionInput *input;
	GtkLabel *output_label;
};

G_DEFINE_TYPE(BoolToolApplicationWindow, bool_tool_application_window, GTK_TYPE_APPLICATION_WINDOW)

static void bool_tool_application_window_dispose(GObject *gobject) {
	gtk_widget_dispose_template(GTK_WIDGET(gobject), BOOL_TOOL_APPLICATION_WINDOW_TYPE);

	G_OBJECT_CLASS(bool_tool_application_window_parent_class)->dispose(gobject);
}

static void bool_tool_application_window_class_init(BoolToolApplicationWindowClass *class) {
	G_OBJECT_CLASS(class)->dispose = bool_tool_application_window_dispose;

	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/com/github/TarekSaeed0/bool_tool/ui/application_window.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, BoolToolApplicationWindow, input);
	gtk_widget_class_bind_template_child(widget_class, BoolToolApplicationWindow, output_label);
}

static void bool_tool_application_window_init(BoolToolApplicationWindow *self) {
	gtk_widget_init_template(GTK_WIDGET(self));
}

BoolToolApplicationWindow *bool_tool_application_window_new(BoolToolApplication *application) {
	return g_object_new(BOOL_TOOL_APPLICATION_WINDOW_TYPE, "application", application, NULL);
}

void simplify_button_clicked(GtkButton *self, gpointer user_data) {
	(void)user_data;

	BoolToolApplicationWindow *window = BOOL_TOOL_APPLICATION_WINDOW(
		gtk_widget_get_ancestor(GTK_WIDGET(self), BOOL_TOOL_APPLICATION_WINDOW_TYPE)
	);

	struct boolean_function function = boolean_function_input_get_function(window->input);

	struct boolean_minterms minterms;
	struct boolean_variables variables;
	switch (function.representation) {
		case boolean_function_representation_expression: {
			minterms = boolean_minterms_from_expression(&function.expression);
			variables = boolean_variables_from_expression(&function.expression);
			boolean_expression_drop(&function.expression);
		} break;
		case boolean_function_representation_minterms: {
			minterms = function.minterms;
			variables = function.variables;
		} break;
	}

	struct boolean_implicants implicants = boolean_implicants_from_minterms(&minterms, &variables);

	boolean_implicants_minimalize(&implicants, &minterms);
	boolean_minterms_drop(&minterms);

	struct boolean_expression expression =
		boolean_expression_from_implicants(&implicants, &variables);
	boolean_implicants_drop(&implicants);
	boolean_variables_drop(&variables);

	char *expression_string = boolean_expression_to_string(&expression);
	gtk_label_set_text(window->output_label, expression_string);
	free(expression_string);

	boolean_expression_drop(&expression);
}

void input_changed(BooleanFunctionInput *self, gpointer user_data) {
	(void)user_data;

	BoolToolApplicationWindow *window = BOOL_TOOL_APPLICATION_WINDOW(
		gtk_widget_get_ancestor(GTK_WIDGET(self), BOOL_TOOL_APPLICATION_WINDOW_TYPE)
	);

	gtk_label_set_text(window->output_label, "");

	g_print("TEST!\n");
}
