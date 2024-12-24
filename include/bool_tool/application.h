#ifndef BOOL_TOOL_APPLICATION_H
#define BOOL_TOOL_APPLICATION_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BOOL_TOOL_APPLICATION_TYPE bool_tool_application_get_type()
G_DECLARE_FINAL_TYPE(BoolToolApplication, bool_tool_application, BOOL_TOOL, APPLICATION, GtkApplication)

BoolToolApplication *bool_tool_application_new(void);

G_END_DECLS

#endif
