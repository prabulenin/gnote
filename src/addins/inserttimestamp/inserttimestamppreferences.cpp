/*
 * gnote
 *
 * Copyright (C) 2011-2013 Aurimas Cernius
 * Copyright (C) 2009 Hubert Figuiere
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <string>

#include <glibmm/i18n.h>

#include "sharp/datetime.hpp"
#include "sharp/propertyeditor.hpp"

#include "preferences.hpp"
#include "inserttimestamppreferences.hpp"
 
using gnote::Preferences;

namespace inserttimestamp {

  const char * SCHEMA_INSERT_TIMESTAMP = "org.gnome.gnote.insert-timestamp";
  const char * INSERT_TIMESTAMP_FORMAT = "format";

  bool InsertTimestampPreferences::s_static_inited = false;
  std::vector<std::string> InsertTimestampPreferences::s_formats;

  void InsertTimestampPreferences::_init_static()
  {
    if(!s_static_inited) {
      s_formats.push_back("%c");
      s_formats.push_back("%m/%d/%y %H:%M:%S");
      s_formats.push_back("%m/%d/%y");
      s_formats.push_back("%Y-%m-%d %H:%M:%S");
      s_formats.push_back("%Y-%m-%d");

      s_static_inited = true;
    }
  }


  InsertTimestampPreferences::InsertTimestampPreferences(gnote::NoteManager &)
  {
    _init_static();

    set_row_spacing(12);
    int row = 0;

    // Get current values
    Glib::RefPtr<Gio::Settings> settings = Preferences::obj().get_schema_settings(SCHEMA_INSERT_TIMESTAMP);
    std::string dateFormat = settings->get_string(INSERT_TIMESTAMP_FORMAT);

    sharp::DateTime now = sharp::DateTime::now();

    // Label
    Gtk::Label *label = manage(new Gtk::Label (_("Choose one of the predefined formats "
                                                 "or use your own.")));
    label->property_wrap() = true;
    label->property_xalign() = 0;
    attach(*label, 0, row++, 1, 1);

    // Use Selected Format
    Gtk::RadioButtonGroup group;
    selected_radio = manage(new Gtk::RadioButton (group, _("Use _Selected Format"), true));
    attach(*selected_radio, 0, row++, 1, 1);

    // 1st column (visible): formatted date
    // 2nd column (not visible): date format
    store = Gtk::ListStore::create(m_columns);

    for(std::vector<std::string>::const_iterator iter = s_formats.begin();
        iter != s_formats.end(); ++iter) {

      Gtk::TreeIter treeiter = store->append();
      treeiter->set_value(0, now.to_string(*iter));
      treeiter->set_value(1, *iter);
    }

    scroll = manage(new Gtk::ScrolledWindow());
    scroll->set_shadow_type(Gtk::SHADOW_IN);
    scroll->set_hexpand(true);
    scroll->set_vexpand(true);
    attach(*scroll, 0, row++, 1, 1);

    tv = manage(new Gtk::TreeView (store));
    tv->set_headers_visible(false);
    tv->append_column ("Format", m_columns.formatted);

    scroll->add (*tv);

    // Use Custom Format
    Gtk::Grid *customBox = manage(new Gtk::Grid);
    customBox->set_column_spacing(12);
    attach(*customBox, 0, row++, 1, 1);

    custom_radio = manage(new Gtk::RadioButton (group, _("_Use Custom Format"), true));
    customBox->attach(*custom_radio, 0, 0, 1, 1);

    custom_entry = manage(new Gtk::Entry());
    customBox->attach(*custom_entry, 1, 0, 1, 1);

    sharp::PropertyEditor *  entryEditor = new sharp::PropertyEditor(
      settings, INSERT_TIMESTAMP_FORMAT, *custom_entry);
    entryEditor->setup ();

    // Activate/deactivate widgets
    bool useCustom = true;
    Gtk::TreeIter iter;
    for(iter = store->children().begin();
        iter != store->children().end(); ++iter) {

      const Gtk::TreeRow & tree_row(*iter);
      std::string value = tree_row[m_columns.format];
      if (dateFormat == value) {
        // Found format in list
        useCustom = false;
        break;
      }
    }

    if (useCustom) {
      custom_radio->set_active(true);
      scroll->set_sensitive(false);
    } 
    else {
      selected_radio->set_active(true);
      custom_entry->set_sensitive(false);
      tv->get_selection()->select(iter);
      Gtk::TreePath treepath = store->get_path (iter);				
      tv->scroll_to_row(treepath);
    }

    // Register Toggled event for one radio button only
    selected_radio->signal_toggled().connect(
      sigc::mem_fun(*this, 
                    &InsertTimestampPreferences::on_selected_radio_toggled));
    tv->get_selection()->signal_changed().connect(
      sigc::mem_fun(*this, 
                    &InsertTimestampPreferences::on_selection_changed));
    show_all ();
  }


  /// Called when toggling between radio buttons.
  /// Activate/deactivate widgets depending on selection.
  void InsertTimestampPreferences::on_selected_radio_toggled ()
  {
    if (selected_radio->get_active()) {
      scroll->set_sensitive(true);
      custom_entry->set_sensitive(false);
      // select 1st row
      Gtk::TreeIter iter;
      iter = store->children().begin();
      tv->get_selection()->select(iter);
      Gtk::TreePath treepath = store->get_path(iter);				
      tv->scroll_to_row(treepath);
    } 
    else {
      scroll->set_sensitive(false);
      custom_entry->set_sensitive(true);
      tv->get_selection()->unselect_all ();
    }
  }

  /// Called when a different format is selected in the TreeView.
  /// Set the GConf key to selected format.
  void InsertTimestampPreferences::on_selection_changed ()
  {
    Gtk::TreeIter iter;

    iter = tv->get_selection()->get_selected();
    if (iter) {
      std::string format;
      iter->get_value(1, format);
      Preferences::obj().get_schema_settings(SCHEMA_INSERT_TIMESTAMP)->set_string(
          INSERT_TIMESTAMP_FORMAT, format);
    }
  }

}
