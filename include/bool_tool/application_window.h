#ifndef BOOL_TOOL_APPLICATION_WINDOW_H
#define BOOL_TOOL_APPLICATION_WINDOW_H

#include <bool_tool/application.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BOOL_TOOL_APPLICATION_WINDOW_TYPE bool_tool_application_window_get_type()
G_DECLARE_FINAL_TYPE(
	BoolToolApplicationWindow,
	bool_tool_application_window,
	BOOL_TOOL,
	APPLICATION_WINDOW,
	GtkApplicationWindow
)

BoolToolApplicationWindow *bool_tool_application_window_new(BoolToolApplication *application);

G_END_DECLS

#endif
