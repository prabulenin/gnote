/*
 * gnote
 *
 * Copyright (C) 2010-2013 Aurimas Cernius
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



#ifndef _TRAYICON_HPP_
#define _TRAYICON_HPP_

#include <gtkmm/statusicon.h>
#include <gtkmm/imagemenuitem.h>

#include "base/macros.hpp"
#include "note.hpp"

#ifdef HAVE_X11_SUPPORT
#include "keybinder.hpp"
#endif

namespace gnote {

class TrayIcon;
class PrefsKeybinder;
class NoteManager;

class NoteMenuItem 
  : public Gtk::ImageMenuItem
{
public:
  NoteMenuItem(const Note::Ptr & note, bool show_pin);

protected:
  virtual void on_activate() override;
  virtual bool on_button_press_event(GdkEventButton *) override;
  virtual bool on_button_release_event(GdkEventButton *) override;
  virtual bool on_motion_notify_event(GdkEventMotion *) override;
  virtual bool on_leave_notify_event(GdkEventCrossing *) override;

private:
  Note::Ptr   m_note;
  Gtk::Image *m_pin_img;
  bool        m_pinned;
  bool        m_inhibit_activate;

  static std::string get_display_name(const Note::Ptr & n);
  static std::string ellipsify (const std::string & str, size_t max);
  static Glib::RefPtr<Gdk::Pixbuf> get_pinup_icon();
  static Glib::RefPtr<Gdk::Pixbuf> get_pindown_icon();
};


class IGnoteTray
{
public:
  virtual void show_menu(bool select_first_item) = 0;
  virtual bool menu_opens_upward() = 0;
};

class Tray
{
public:
  typedef shared_ptr<Tray> Ptr;
#ifdef HAVE_X11_SUPPORT
  Tray(NoteManager &, IGnoteTray &, IKeybinder &);
#else
  Tray(NoteManager &, IGnoteTray &);
#endif

  Gtk::Menu * make_tray_notes_menu();
  Gtk::Menu * tray_menu() 
    { return m_tray_menu; }
  void update_tray_menu(Gtk::Widget * parent);
  void add_recently_changed_notes();
private:
  void on_new_note_activate();
  void on_search_notes_activate();

  NoteManager & m_manager;
  IGnoteTray  & m_trayicon;
  Gtk::Menu *m_tray_menu;
  Gtk::ImageMenuItem *m_new_note_item;
  Gtk::ImageMenuItem *m_search_notes_item;
  bool       m_menu_added;
#ifdef HAVE_X11_SUPPORT
  IKeybinder & m_keybinder;
#endif
};


class TrayIcon
  : public Gtk::StatusIcon
  , public IGnoteTray
{
public:
#ifdef HAVE_X11_SUPPORT
  TrayIcon(IKeybinder & keybinder, NoteManager & manager);
#else
  explicit TrayIcon(NoteManager & manager);
#endif
  ~TrayIcon();

  Tray::Ptr tray() const
    { return m_tray; }

  void show_menu(bool select_first_item);

  Gtk::Menu * get_right_click_menu();

  void on_activate();
  void on_popup_menu(guint button, guint32 activate_time);
  void on_exit();
  bool menu_opens_upward();

  void show_preferences();
  void show_help_contents();
  void show_about();
  void quit();
protected:
  virtual bool on_size_changed(int size) override;
private:
  Tray::Ptr                m_tray;
  Gtk::Menu               *m_context_menu;
#ifdef HAVE_X11_SUPPORT
  PrefsKeybinder          *m_keybinder;
#endif
};


#ifdef HAVE_X11_SUPPORT
class KeybindingToAccel
{
public:
  static std::string get_shortcut (const std::string & key);
  static void add_accelerator(IKeybinder & keybinder, Gtk::MenuItem & item, const std::string & key);

  static Glib::RefPtr<Gtk::AccelGroup> get_accel_group();
private:
  static Glib::RefPtr<Gtk::AccelGroup> s_accel_group;
};
#endif



std::string tray_util_get_tooltip_text();

}
#endif
