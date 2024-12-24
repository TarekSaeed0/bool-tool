#include <bool_tool/application.h>

#include <bool_tool/application_window.h>
#include <bool_tool/boolean_function_input.h>
#include <glib-object.h>

struct _BoolToolApplication {
	GtkApplication parent;
};

G_DEFINE_TYPE(BoolToolApplication, bool_tool_application, GTK_TYPE_APPLICATION)

static void bool_tool_application_init(BoolToolApplication *self) {
	(void)self;
}

static void remove_style_provider(gpointer data) {
	GtkStyleProvider *provider = GTK_STYLE_PROVIDER(data);
	gtk_style_context_remove_provider_for_display(gdk_display_get_default(), provider);
}

static void bool_tool_application_activate(GApplication *application) {
	g_type_ensure(BOOLEAN_FUNCTION_INPUT_TYPE);
	g_type_ensure(BOOL_TOOL_APPLICATION_WINDOW_TYPE);

	BoolToolApplicationWindow *window =
		bool_tool_application_window_new(BOOL_TOOL_APPLICATION(application));

	GtkCssProvider *provider = gtk_css_provider_new();
	gtk_css_provider_load_from_resource(provider, "/com/github/TarekSaeed0/bool_tool/style.css");
	gtk_style_context_add_provider_for_display(
		gdk_display_get_default(),
		GTK_STYLE_PROVIDER(provider),
		G_MAXUINT
	);
	g_object_set_data_full(
		G_OBJECT(window),
		"provider",
		GTK_STYLE_PROVIDER(provider),
		remove_style_provider
	);

	GtkIconTheme *icon_theme =
		gtk_icon_theme_get_for_display(gtk_widget_get_display(GTK_WIDGET(window)));
	gtk_icon_theme_add_resource_path(icon_theme, "/com/github/TarekSaeed0/bool_tool/icons/hicolor");

	gtk_window_present(GTK_WINDOW(window));
}

static void bool_tool_application_class_init(BoolToolApplicationClass *class) {
	G_APPLICATION_CLASS(class)->activate = bool_tool_application_activate;
}

BoolToolApplication *bool_tool_application_new(void) {
	return g_object_new(
		BOOL_TOOL_APPLICATION_TYPE,
		"application-id",
		"com.github.TarekSaeed0.bool_tool",
		"flags",
		G_APPLICATION_DEFAULT_FLAGS,
		NULL
	);
}
