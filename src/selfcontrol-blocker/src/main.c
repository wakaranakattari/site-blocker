#include "blocker.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  GtkWidget *window;
  GtkWidget *entry;
  GtkWidget *list;
  GtkListStore *store;
  GtkWidget *btn_add;
  GtkWidget *btn_remove;
  GtkWidget *status;
} AppData;

static void
update_site_list (AppData *app)
{
  gtk_list_store_clear (app->store);

  char sites[MAX_SITES][MAX_DOMAIN_LEN];
  int count = get_blocked_sites (sites, MAX_SITES);

  for (int i = 0; i < count; i++)
    {
      GtkTreeIter iter;
      gtk_list_store_append (app->store, &iter);
      gtk_list_store_set (app->store, &iter, 0, sites[i], -1);
    }

  char status_text[256];
  snprintf (status_text, sizeof (status_text), "Blocked sites: %d", count);
  gtk_label_set_text (GTK_LABEL (app->status), status_text);
}

static void
on_add_clicked (GtkWidget *widget, AppData *app)
{
  (void)widget;

  const char *input = gtk_entry_get_text (GTK_ENTRY (app->entry));
  if (strlen (input) > 0)
    {
      if (add_site_to_block (input))
        {
          update_site_list (app);
          gtk_entry_set_text (GTK_ENTRY (app->entry), "");

          GtkWidget *dialog = gtk_message_dialog_new (
              GTK_WINDOW (app->window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
              GTK_BUTTONS_OK, "Site blocked successfully");
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
        }
      else
        {
          GtkWidget *dialog = gtk_message_dialog_new (
              GTK_WINDOW (app->window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
              GTK_BUTTONS_OK, "Failed to block site");
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
        }
    }
}

static void
on_remove_clicked (GtkWidget *widget, AppData *app)
{
  (void)widget;

  GtkTreeSelection *selection
      = gtk_tree_view_get_selection (GTK_TREE_VIEW (app->list));
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      char *domain;
      gtk_tree_model_get (model, &iter, 0, &domain, -1);

      if (remove_site_from_block (domain))
        {
          update_site_list (app);

          GtkWidget *dialog = gtk_message_dialog_new (
              GTK_WINDOW (app->window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
              GTK_BUTTONS_OK, "Site removed successfully");
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
        }
      else
        {
          GtkWidget *dialog = gtk_message_dialog_new (
              GTK_WINDOW (app->window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
              GTK_BUTTONS_OK, "Failed to remove site");
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
        }

      g_free (domain);
    }
}

static void
init_gui (AppData *app)
{
  app->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (app->window), "Site Blocker");
  gtk_window_set_default_size (GTK_WINDOW (app->window), 500, 500);
  gtk_window_set_position (GTK_WINDOW (app->window), GTK_WIN_POS_CENTER);
  g_signal_connect (app->window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);
  gtk_container_add (GTK_CONTAINER (app->window), vbox);

  GtkWidget *label = gtk_label_new ("Site Blocker");
  gtk_label_set_markup (
      GTK_LABEL (label),
      "<span size='large' weight='bold'>Site Blocker</span>");
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 5);

  app->status = gtk_label_new ("Blocked sites: 0");
  gtk_box_pack_start (GTK_BOX (vbox), app->status, FALSE, FALSE, 5);

  GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 5);

  app->entry = gtk_entry_new ();
  gtk_entry_set_placeholder_text (
      GTK_ENTRY (app->entry),
      "Enter domain or URL (e.g., example.com or https://example.com/page)");
  gtk_box_pack_start (GTK_BOX (hbox), app->entry, TRUE, TRUE, 5);

  app->btn_add = gtk_button_new_with_label ("Block");
  g_signal_connect (app->btn_add, "clicked", G_CALLBACK (on_add_clicked), app);
  gtk_box_pack_start (GTK_BOX (hbox), app->btn_add, FALSE, FALSE, 5);

  GtkWidget *scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), scrolled, TRUE, TRUE, 5);

  app->store = gtk_list_store_new (1, G_TYPE_STRING);
  app->list = gtk_tree_view_new_with_model (GTK_TREE_MODEL (app->store));

  GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (
      "Blocked Sites", renderer, "text", 0, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (app->list), column);

  gtk_container_add (GTK_CONTAINER (scrolled), app->list);

  app->btn_remove = gtk_button_new_with_label ("Remove Selected");
  g_signal_connect (app->btn_remove, "clicked", G_CALLBACK (on_remove_clicked),
                    app);
  gtk_box_pack_start (GTK_BOX (vbox), app->btn_remove, FALSE, FALSE, 5);

  gtk_widget_show_all (app->window);
}

int
main (int argc, char *argv[])
{
  if (geteuid () != 0)
    {
      g_print ("This program requires root privileges to modify /etc/hosts\n");
      g_print ("Run with sudo: sudo %s\n", argv[0]);
      return 1;
    }

  gtk_init (&argc, &argv);

  AppData *app = g_malloc0 (sizeof (AppData));

  init_gui (app);
  update_site_list (app);

  gtk_main ();

  g_free (app);

  return 0;
}
