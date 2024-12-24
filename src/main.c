#include <gtk/gtk.h>

#include <bool_tool/application.h>

int main(int argc, char *argv[]) {
	// Disable client-side decorations
	putenv((char[]){ "GTK_CSD=0" });

	return g_application_run(G_APPLICATION(bool_tool_application_new()), argc, argv);
}
