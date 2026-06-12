#include <gtk/gtk.h>
#include "gui.h"
#include "history.h"

static void on_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    create_main_window(app);
}

int main(int argc, char *argv[]) {
    // Initialize persistent calculation history
    history_init();
    
    // Create GtkApplication instance
    GtkApplication *app = gtk_application_new("com.antigravity.calc", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    
    // Run the app loop
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    
    // Cleanup and save history data
    g_object_unref(app);
    history_free();
    
    return status;
}
